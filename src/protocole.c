#include "protocole.h"
#include "tlv.h"
#include "pair.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdint.h>
#include "struct_data.h"
#include <limits.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <locale.h>
#include <sys/random.h>


#define htonll(x) (((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) (((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

//init table voisin avec state à -1 pour savoir qu'elle est vide
void init_tab(t_table_voisin *t){
	int i;
	for(i=0;i<15;i++){
		t[i].state=-1;
	}
}

//vérifie l'entete
int check_entete(datagramme paquet){
	//printf("magic %d version %d length %d \n",paquet.Magic,paquet.Version,htons(paquet.Body_length));
	
	
	if( (paquet.Magic!=95) || (paquet.Version!=1) || (htons(paquet.Body_length)>1020) )
		return 0;
	else
		return 1;
}
	

//affiche dans la console le char en binaire
void printbincharpad(char c)
{
    for (int i = 7; i >= 0; --i)
    {
        putchar( (c & (1 << i)) ? '1' : '0' );
    }
    putchar(' ');
}

//appel la fonction précédente pour afficher le contenu du paquet en binaire (de 8 en 8)
void print_buff(void *buff, int len) {
	int i;
	printf("len : %d \n",len);
	for(i = 0;i < len;i++) {
		printbincharpad(((char *)buff)[i]);
	}
	printf("\n");
}

//affiche le message d'un warning si on en recoit un
void readWarning(int len,unsigned char *buff) {
	printf("%.*s \n",htons(len)-2,buff+6);
	exit(1);
}

//envoie un paquet request voisin et retourne les infos demandées
 void ask_voisin(struct in6_addr addr,uint16_t port,int s){
	//int to;
	t_req req={0};
	req.data.Magic=95;
	req.data.Version=1;
	req.data.Body_length=htons(2);
	t_TLV *tlv=(void*)req.data.body;
	tlv->voisin_req.Type=2;
    tlv->voisin_req.Length=htons(0);				
	char ipstr[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &addr, ipstr, sizeof ipstr);
//	printf("->j'envoie une demande de nouveau voisin à %s\n",ipstr);
	client(addr,port,s,req.buff, htons(req.data.Body_length)+4);
	return;		
}

void hello_new_neigbhour(t_Neighbour voisin,int s,t_table_donnee *td){
	//printf("->j'envoie un coucou au nouveau voisin\n");
	send_neth(td,voisin.ip,voisin.Port,s);
	
}
	

//met à jour la table des voisins
void majTab_voisin(uint16_t port,struct in6_addr addr,char *ip,time_t lastseen,t_table_voisin *t) {

	int i;
	if((i=searchTab(t,ip))!=-1){
		t[i].lastseen=lastseen;
		return;
	}
	for(i=0;i<15;i++){
		if(t[i].state==-1){
			if(strcmp(ip,ADD4)==0 || strcmp(ip,ADD6)==0) {
				t[i].state=TRUE;
				strcpy(t[i].ip, ip);
				t[i].addr=addr;	
				t[i].port=port;
				t[i].lastseen=lastseen;
				return;
			}
			else{
				t[i].state=FALSE;
				strcpy(t[i].ip, ip);
				t[i].addr=addr;
				t[i].port=port;
				t[i].lastseen=lastseen;
				return;
			}
		}
	}
	return;

}
//on cherche si l'emetteur est dans la table
//le premier découvert a pour état vrai(constant)
int searchTab(t_table_voisin *t,char *ip) {
	int i;
	for(i=0; i<15; i++) {
		if(t[i].state !=-1 ) {
			if(strcmp(ip,t[i].ip) == 0 ) {
				return i;
			}
		}
	}
	return -1;
		
}

		
//parcours cette table toutes les 20secondes 
//si un voisin non constant n'a pas émis de paquet depuis 70s, il est eliminé de la table voisin
void parcours(t_table_voisin *t){
	int i;
	long s;
	for(i=0; i<15; i++){
		if(t[i].state!=-1){
			s=(unsigned long) difftime(time(NULL),t[i].lastseen);
			printf("time_t %lu\n",s);
			if(s>70 && t[i].state!=1){
				printf("desolé %i t'étais plus là\n",i);
				t[i].state=-1;
			}
		}
	}	
	
}


void interprete_node_state(t_Node_state n_state,t_table_donnee *td){
			int len=td->len;
			uint64_t id=ntohll(n_state.node_id);
			Node node={0};
			int taille=n_state.Length-26;
			if(taille>192)
				return;
			int i;
			for(i=0;i<len; i++){
				if(id==td->n[i].id){
					if(memcmp(n_state.Node_Hash,h16b(td->n+i),16)==0)
						return;
					 else if(id==td->id){
					 	comp_seq(td,i,n_state.Seqno);
					 	return;
					 }
					 break;
				}
			}
			node.id=id;
			node.s=n_state.Seqno;
			memcpy(node.data,n_state.Data,taille);
			maj_tab_node(node,td);
					
}

//vérifie pour chaque node hash recu dans le paquet ses id ,sinon envoi une 
//node state request
void search_nodh(t_Node_hash nh,t_table_donnee *td,struct in6_addr addr,uint16_t port,int s){
	int i;
	uint64_t id;
	Node *n=td->n;
    int len=td->len;
	for(i=0;i<len;i++){
		id=ntohll(nh.node_id);
		if(id==n[i].id){
			if(memcmp(h16b(&n[i]),nh.node_h,16)==0){
				return;
			}
		}
		else{
			t_req req={0};
			req.data.Magic=95;
			req.data.Version=1;
			req.data.Body_length=htons(10);
			t_TLV * tlv=(void *)req.data.body;
			tlv->node_statereq.Type=7;
			tlv->node_statereq.Length=8;
			tlv->node_statereq.node_id=id;
			client(addr,port,s,req.buff,14);
			return;	
		}
	}
	return;
}

void send_warning(struct in6_addr addr,int s,uint16_t port,char *message){
	t_req req={0};
	req.data.Magic=95;
	req.data.Version=1;
	req.data.Body_length=htons(2+strlen(message));
	t_TLV *tlv=tlv=(void *)req.data.body;
	tlv->warning.Type=9;
	tlv->warning.Length=strlen(message);
	//tlv->warning.Message=malloc(strlen(message)*sizeof(char));
	memcpy(tlv->warning.Message,message,strlen(message));
	client(addr,port,s,req.buff,htons(req.data.Body_length));
}
	
//lit le paquet recu 
void read_tlv(t_req recu,t_table_donnee *td,struct in6_addr addr,int s,uint16_t port,t_table_voisin *t){
	int len=htons(recu.data.Body_length);
	int i=0;
	t_TLV *TLV;
	//printf("\n\n--- TLV recus: %d bytes---\n\n",len);
	while(i<len){
		TLV=((void *)recu.data.body)+i;
		if(TLV->tlv.Length>1024-i)
			send_warning(addr,s,port,"mauvaise len tlv");
		//printf("Type: %d\n",TLV->tlv.Type);
		if(TLV->tlv.Type==0){
			i=i+1;
		}
		else{
			interprete_type(TLV->tlv.Type,TLV,td,addr,s,port,t,recu);
			i=i+TLV->tlv.Length+2;
		}
		
	}
	return;
	//interprete_type(recu.data.body[i].tlv.Type,recu,i,td,ip,s,port,t);
	
}

//compte le nombre de voisins		
int nb_voisin(t_table_voisin *t){
	int i,rep;
	rep=0;
	for(i=0;i<15;i++){
		if(t[i].state!=-1)
			rep++;
	}
	return rep;
}

//envoie un node state enfonction de l'id demandé 
void send_state(struct in6_addr addr,uint16_t port,int s,t_table_donnee *td, t_Node_State_Request recu){
	t_req req={0};
	t_TLV *tlv;
	Node n;

	req.data.Magic=95;
	req.data.Version=1;
	req.data.Body_length=htons(0);
	
	tlv = (void*)req.data.body;
	int i = -1;
	while (++i < td->len && td->n[i].id != ntohll(recu.node_id));
	if (i == td->len)
		return ; // If asking for node we don't have
	n = td->n[i];
	tlv->node_state.Type = 8;
    tlv->node_state.Length = 26 + strlen(n.data);
	tlv->node_state.node_id = htonll(n.id);
	tlv->node_state.Seqno = n.s;
	memcpy(tlv->node_state.Node_Hash, h16b(&n), 16 * sizeof(char));
	memcpy(tlv->node_state.Data, n.data, strlen(n.data));
	req.data.Body_length = htons(htons(req.data.Body_length) + tlv->node_state.Length + 2);
	if(ntohll(n.id)==td->id)
	dprintf(2, "%16lx(i == %d): (%d) - %s\n", ntohll(n.id), i, ntohs(n.s), n.data);
	client(addr, port, s, req.buff, htons(req.data.Body_length)+4);

}

//envoi une liste de Node Hash
void send_nh(struct in6_addr addr,uint16_t port,int s,t_table_donnee *td){
	t_req req={0};
	Node n={0};
	req.data.Magic=95;
	req.data.Version=1;
	req.data.Body_length=htons(0);
	int len=td->len;
	int i;
	int j=0;
	t_TLV *tlv;
	char ipstr[INET6_ADDRSTRLEN];
	for(i=0; i<len; i++){
		if(j+26< 1020){
			n=td->n[i];
			tlv=(void *)req.data.body+j;
			tlv->nodeh.Type=6;
			tlv->nodeh.Length=26;
			tlv->nodeh.node_id=htonll(n.id);
			tlv->nodeh.Seqno=n.s;
			memcpy(tlv->nodeh.node_h,h16b(td->n+i),16);
			req.data.Body_length=htons(htons(req.data.Body_length)+tlv->node_state.Length+2);
			j=htons(req.data.Body_length);
		}
		else{
			inet_ntop(AF_INET6, &addr, ipstr, sizeof ipstr);
		   // printf("->j'envoie node hash à %s\n",ipstr);
				client(addr,port,s,req.buff,htons(req.data.Body_length)+4);
			req.data.Body_length=htons(0);
			memset(req.data.body,0,sizeof(tlv));
		    j=0;
			i--;
		}
	}
	if(htons(req.data.Body_length)>0){
		inet_ntop(AF_INET6, &addr, ipstr, sizeof ipstr);
		//printf("->j'envoie node hash à %s\n",ipstr);
		client(addr,port,s,req.buff,htons(req.data.Body_length)+4);	}
	return;
}

//envoie un voisin au hasard
void send_neighbour(struct in6_addr addr,uint16_t port,t_table_voisin *t,int s){
	int num;
	getrandom(&num, sizeof(num), 0);
 	num=num%15;
 	while(t[num].state==-1) {
 		num=(num+1)%15;
 	} 
 	t_req req={0};
 	req.data.Magic=95;
 	req.data.Version=1;
 	t_TLV *tlv=(void *)req.data.body;
 	req.data.Body_length=htons((tlv->voisin.Length)+2);
 	tlv->voisin.Type=3;
 	tlv->voisin.Length=sizeof(tlv->voisin.ip)+sizeof(t[num].port);
 	tlv->voisin.ip=t[num].addr;
 	tlv->voisin.Port=t[num].port;
 	char ipstr[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6,&addr, ipstr, sizeof ipstr);
	//printf("->j'envoie voisin à %s\n",ipstr);
 	client(addr,port,s,req.buff,htons(req.data.Body_length)+4);
 	return;
 }	

//fonction qui permet de réagrir en fonction des tlv reçus
void interprete_type(int t,t_TLV *tlv,t_table_donnee *td,struct in6_addr addr,int s,uint16_t port,t_table_voisin *voisin,t_req recu){
	switch(t){
	//char *h16 (Node n);

	case 0: printf("ignoré : Pad1 \n\n"); break;
	
	case 1: printf("ignoré : PadN \n\n"); break;
	
	case 2: printf("Neighbour request \n\n");send_neighbour(addr,port,voisin,s); break;
	
	case 3: printf("Neighbour \n\n");hello_new_neigbhour(tlv->voisin,s,td);break;
	
	case 4: printf("Network Hash \n\n");
		check_Network_hash(tlv->networkh,td,addr,port,s);
		break;
	
	case 5: printf("Network State request \n\n"); send_nh(addr,port,s,td); break;
	
	case 6: /*printf("Node Hash \n\n")*/;search_nodh(tlv->nodeh,td,addr,port,s); break;
	
	case 7: /*printf("Node State request \n\n");*/ send_state(addr,port,s,td,tlv->node_statereq ); break;
	
	case 8: /*printf("Node State \n\n");*/interprete_node_state(tlv->node_state,td) ;break;
	
	case 9: printf(" Warning \n\n");readWarning(recu.data.Body_length,recu.buff); break;
	
	default: break;
	}
	//printf("--------------------------------------\n\n");
	return;
	
	
}

//vérifie le network hash recu,si différent,envoi une network state request
void check_Network_hash(t_Network_hash nh,t_table_donnee *td,struct in6_addr addr,uint16_t port,int s){
	Node *n=td->n;
	int len=td->len;
	if(memcmp(nh.NetworkHash,networkh(n,len),16)==0)
		return ;
	else{
		t_req envoie={0};
		envoie.data.Magic=95;
		envoie.data.Version=1;
		envoie.data.Body_length=htons(2);
		t_TLV *tlv=(void *)envoie.data.body;
		tlv->network_statereq.Type=5;
		tlv->network_statereq.Length=0;
		char ipstr[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &addr, ipstr, sizeof ipstr);
		//printf("->j'envoie network state request à %s : %d \n",ipstr,port);
		client(addr,port,s,envoie.buff,6);
		}
}

//envoie un network hash
void send_neth(t_table_donnee *td,struct in6_addr addr,uint16_t port,int s){
	t_req envoie={0};
	envoie.data.Magic=95;
	envoie.data.Version=1;
	envoie.data.Body_length=htons(18);
	t_TLV *tlv=(void *)envoie.data.body;
	tlv->networkh.Type=4;
	tlv->networkh.Length=16;
	
	Node *n=td->n;
	int len=td->len;
	memcpy(tlv->networkh.NetworkHash,networkh(n,len),16);
	char ipstr[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &addr, ipstr, sizeof ipstr);
	//dprintf(2,"->j'envoie network hash à %s\n",ipstr);
	client(addr,port,s,envoie.buff,22);
	
	return;

}
	
