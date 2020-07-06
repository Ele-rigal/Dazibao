#include "struct_data.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <endian.h>
#include <time.h>
#include <sys/random.h>
#include <arpa/inet.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define htonll(x) (((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) (((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

uint16_t num_sequence(uint16_t s,int n){
	return (s+htons(n))&65535;
}

//s1<<=s2 ssi ((s2-s1) & 32768)==0
int relat_ternaire(uint16_t s1,uint16_t s2){
	if(((ntohs(s2)-ntohs(s1))&32768)==0)
		return 1;
	else
		return 0;
}

uint64_t gen_id() {
   uint64_t num;
   getrandom(&num, sizeof(num), 0);
    return ntohll(num);
}

char *h16 (Node *n){
	int i;
	//static char hash16[16];
    char * hash16;
    hash16=calloc(1,33);
	unsigned char *d=h16b(n);
	for (i = 0; i < 16; i++) {
		//hash16[i]=d[i];
		sprintf(hash16+i*2,"%02x",d[i]);
		//printf("%02x", d[i]);
	}
	return hash16;
}

unsigned char* h16b(Node *n) {
	//n->s=htons(n->s);
	n->id=htonll(n->id);
 	int len=sizeof(n->id)+sizeof(n->s)+strlen(n->data);
 	unsigned char *d=SHA256(((void *)n),len, 0);
 	static unsigned char sha[16];
 	memcpy(sha,d,16);
 	return sha;
	
}
void print_tab_node(Node *n,int len){
	int i;
	printf("------tab node-----\n");
	for(i=0;i<len;i++){
		printf("---%d---\n",i);
		printf("id : %8lx\n",ntohll(n[i].id));
		printf("s: %d\n",ntohs(n[i].s));
		printf("%s\n",n[i].data);
	}
	printf("\n\n");
}

#define A "🌸"

void change_donnee_perso(t_table_donnee *t){
	char *citat[] = {
		A "De temps en temps\nLes nuages nous reposent\nDe tant regarder la lune",
		A "Mes larmes grésillent\nEn éteignant\nLes braises.",
		A "Rien dans le cri\ndes cigales suggère qu'ils\nsont sur le point de mourir",
		A "De quel arbre en fleur ?\nJe ne sais pas.\nMais quel parfum !",
		A "Viens\nAllons voir la neige\nJusqu'à nous ensevelir !",
		A "Le papillon\nparfumant ses ailes\namoureux de l'orchidée",
		A "Parmi les graffitis\nun nom illuminé\nLe tien",
		 "Oops I did it again",
		A "Assis tranquillement, sans rien faire,\nle printemps arrive,\nl'herbe pousse, par elle - même. " ,
		A"Combien je désire !\ndans ma petite sacoche,\nla lune, et des fleurs",
		A"le printemps s'en va\npleurs des oiseaux et poissons\nles larmes aux yeux" ,
		A"ami, allume le feu\nje vais te montrer quelque chose —\nune boule de neige !" ,
		A "malade en voyage\nmes rêves parcourent seuls\nles champs désolés",
		A "le vent d'automne\nplus blanc que les pierres\nde la colline rocheuse" ,
		A "ô pluie du printemps !\nun saule caresse\nma cape de voyageur... " ,		
	};
	unsigned int num;
	getrandom(&num, sizeof(num), 0);
    num=num%15;
    int i;
    int len=t->len;
    Node *n;
    for(i=0; i<len; i++){
    	n=&t->n[i];
    	if(t->id==n->id){
    		memset(n->data,0,192);
    		strncpy(n->data,citat[num],192);
    		n->s=num_sequence(n->s,1);
    		dprintf(2,"id: %016lx changé : %d - %s\n",n->id,ntohs(n->s),n->data);
    		//print_tab_node(t->n,len);
    		//exit(1);
    		break;
    	}
    }
    write_file(t);
   }
    		


//s1<<=s2 ssi ((s2-s1) & 32768)==0
//compare sequences
void comp_seq(t_table_donnee *t,int i,uint16_t Seqno){
	Node *n=&t->n[i];
	//uint16_t s=n[i].s;
	if(relat_ternaire(n->s,Seqno)==1){
		n->s=num_sequence(Seqno,1);
		dprintf(2,"coucou\n");
	}
	dprintf(2," %d - %s\n",ntohs(n->s),n->data);
	//print_tab_node(t->n,t->len);
	return;
}

void write_file(t_table_donnee *td){
	int fd = open("donnee.txt",O_WRONLY | O_CREAT |O_TRUNC,0644);
	if(fd<0){
		perror("erreur open");
		exit(1);
	}
	dprintf(fd,"%16s %5s %s\n","ID","Seqno","Data");
	int i;
	Node *n=td->n;
	for(i=0; i<td->len; i++){
		dprintf(fd,"%016lx ",n[i].id);
		dprintf(fd,"%5d ",ntohs(n[i].s));
		dprintf(fd,"%s\n",n[i].data);
	}
	close(fd);
}

void maj_tab_node(Node node,t_table_donnee *td){
	int i;
	Node *n=td->n;
	int len=td->len;
	//node.s=ntohs(node.s);
	for(i=0;i<len;i++) {
		if (node.id == td->id)
			return ;
		if (n[i].id==node.id)
			break;
	}
	if(i==len){
		td->n=realloc(td->n,(len+1)*sizeof(*td->n));
		td->len=td->len+1;
		td->n[i]=node;
		write_file(td);
		return;
		
	}
	if(relat_ternaire(node.s,n[i].s)==0){
		td->n[i]=node;
		write_file(td);
		return;
	}
	
	return;
}

void init(Node *n){
	n->id=gen_id();
	printf("id: %8lx \n",n->id);
	n->s=0;
}


int partition(Node *tab,int begin,int end,int pivot) {
	Node temp;
	int i,j;
	temp=tab[pivot];
	tab[pivot]=tab[end];
	tab[end]=tab[pivot];
	j=begin;
	for(i=begin;i<end;i++){
		if(tab[i].id<=tab[end].id){
			temp=tab[i];
			tab[i]=tab[j];
			tab[j]=temp;
			j++;
		}
	}
	temp=tab[end];
	tab[end]=tab[j];
	tab[j]=temp;
	return j;
}

void quick_sort(Node *tab,int begin,int end) {
	int pivot;
	if(begin<end){
		pivot=end;
		pivot=partition(tab,begin,end,pivot);
		quick_sort(tab,begin,pivot-1);
		quick_sort(tab,pivot+1,end);
	}
}

unsigned char *networkh(Node *n,int len){
	quick_sort(n,0,len-1);
	printf("len tab node %d\n",len);
	char concath[len*16];
	int i;
	for(i=0; i<len; i++) {
		memcpy(concath+i*16,h16b(n+i),16);
	}
	unsigned char*hash=SHA256((void*)concath,sizeof(concath), 0);
	static unsigned char sha[16];
 	memcpy(sha,hash,16);
 	return sha;
	
}
	

