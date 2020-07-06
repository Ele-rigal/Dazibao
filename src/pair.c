#include "pair.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <poll.h>
#include <time.h>
#include <sys/types.h> 
#include <openssl/sha.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netdb.h>
#include "tlv.h"
#include "protocole.h"
#include <locale.h>
#include "struct_data.h"
#include <fcntl.h>
#include <sys/random.h>
#include <errno.h>
#include <signal.h>

t_table_donnee *td;

void write_file_v(t_table_voisin t[15]){
	int fd = open("voisin.txt",O_WRONLY | O_CREAT |O_TRUNC,0644);
	if(fd<0){
		perror("erreur open");
		exit(1);
	}
	dprintf(fd,"%20s %*s %5s %s\n","Lastseen",INET6_ADDRSTRLEN,"adresse","Port","Etat");
	int i;
	for(i=0; i<15; i++){
		if(t[i].state!=-1){
			dprintf(fd,"%20ld ",t[i].lastseen);
			dprintf(fd,"%*s ",INET6_ADDRSTRLEN,t[i].ip);
			dprintf(fd,"%5d ",t[i].port);
			if(t[i].state==1)
				dprintf(fd,"%s\n","permanent");
			else
				dprintf(fd,"%s\n","transitoire");
		}
	}
	
	close(fd);
}				     
					  
void printTable(t_table_voisin t[15]){
	printf("\n --- TABLE VOISIN --- \n");
	for(int i=0;i<15;i++){
		if(t[i].state!=-1) {
			printf("----%d----\n",i);
			printf("state: %d\n",t[i].state);
			printf("ip: %s\n",t[i].ip);
			printf("port: %d\n",t[i].port);
			printf("lastseen: %ld\n",t[i].lastseen);
		}
		
	}
	printf("\n");
}

//vérifie l'état des sockets
//on utilise poll (mais on aurait pu tout autant utiliser select
/*
POLLIN: données en attente de lecture
POLLPRI:donnéees urgentes en attente de lecture
POLLOUT: une ecriture ne bloquera pas
POLLHUP: indique que la socket a été déconnecté (je crois,mais utile pour udp?)
POLLNVAL: requete invalide: fd n'est pas ouvert(uniquement en sortie
*/

void check_server(int sock,t_table_voisin *t,t_table_donnee *td) {
	int ready;
	int rc;
	//char in[1024]; //1024 taille max paquet ici
	struct pollfd fds[1]; 
	t_req rep={0};
	time_t delay=0;
	struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
	unsigned int num;
	
	/*initialisation du tableau de descripteur*/
	/* on rempli avec le principe de décalage de bytes pour events*/
	fds[0].fd=sock;
	fds[0].events= POLLIN;
	struct in6_addr addr;
	rc=inet_pton(AF_INET6,ADD6,&addr);
	if(rc<1)
		abort();
	majTab_voisin(1212,addr,ADD6,time(NULL),t);
	 write_file_v(t);
	//send_neth(td,addr,1212,sock);
	rc=inet_pton(AF_INET6,ADD4,&addr);
	if(rc<1)
		abort();
	//struct in6_addr addr_m;
	//rc=inet_pton(AF_INET6,"ff12::4eeb:8d51:534e:e69b",&addr_m);
	majTab_voisin(1212,addr,ADD4,time(NULL),t);
	write_file_v(t);
	ask_voisin(addr,1212,sock);
	//send_neth(td,addr,1212,sock);
	while(1) {
		 if(difftime(time(NULL),delay)>20){
		 	getrandom(&num, sizeof(num), 0);
		 	num=num%15;
		 	while(t[num].state==-1) {
		 		num=(num+1)%15;
		 	} 	
		 	if(nb_voisin(t)<5){	
				ask_voisin(t[num].addr,t[num].port, sock);
		 	}
		 	parcours(t);
		 	write_file_v(t);
		 	int compt;
		 	change_donnee_perso(td)	;
		 	for(compt=0;compt<15;compt++){
		 		if(t[compt].state!=-1){
		 			send_neth(td,t[compt].addr,t[compt].port,sock);
		 			
		 			//send_neth(td,addr_m,1212,sock);
		 		}
		 	}	
		 	
			delay=time(NULL);
		}
		ready= poll(fds,1,10);
		if( ready < 0) {
			perror("error poll");
			break;
		}
		if(ready > 0) {
			if(fds[0].revents & (POLLERR | POLLNVAL)) {
			perror("erreur poll socket");
			break;
			}
		  
			if(fds[0].revents & POLLIN) {
				peer_addr_len = sizeof(struct sockaddr_storage);
				rc = recvfrom(sock,rep.buff,1024,0,(struct sockaddr *) &peer_addr,&peer_addr_len);
				if( rc 
				< 0){
					perror("erreur recvfrom");
					break;
				}
				if( rc > 0) {	
					 struct sockaddr_in6 *ip=(void*)&peer_addr;
				     char ipstr[INET6_ADDRSTRLEN];
				     				     inet_ntop(AF_INET6,&ip->sin6_addr,ipstr,sizeof(ipstr));
				     majTab_voisin(htons(ip->sin6_port),ip->sin6_addr,ipstr,time (NULL),t);
				     write_file_v(t);
					if(check_entete(rep.data)==0){
						send_warning(ip->sin6_addr,sock,htons(ip->sin6_port),"mauvaise en-tete");
					}
					else{
						//printf("---message recu!!!---\n");
						//printf("de: %s : %d\n",ipstr,htons(ip->sin6_port));
					     read_tlv(rep,td,ip->sin6_addr,sock,htons(ip->sin6_port),t);
    				 }	
    				
				}
			}
		 }
	   }
	
}
	

//server 
void server(t_table_voisin *t,t_table_donnee *td){
	int s,m;
	if((s=socket(AF_INET6, SOCK_DGRAM, 0))<0) {
		perror("erreur socket");
	}
	if((m=fcntl(s,F_GETFL))<0)
		perror("erreur 2 socket ");
	if(fcntl(s,F_SETFL,m )<0)
		perror("erreur 3 socket");
	int val=0;
	if(setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof(val))<0)
		perror("erreur 4 socket");
	check_server(s,t,td);

}
//client
 void client(struct in6_addr addr,uint16_t port,int s,unsigned char *buff,int len){
	struct sockaddr_in6 server;
	memset(&server, 0, sizeof(server));
	server.sin6_family = AF_INET6;
	server.sin6_addr=addr;
	server.sin6_port=htons(port);
	//printf("len buffer envoyé : %d\n",len);
	if(sendto(s,buff,len,0,((struct sockaddr * )&server), sizeof(server))<0){
		char ipstr[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &addr, ipstr, sizeof ipstr);
		 // printf("->j'envoie node state à %s\n",ipstr);
		 printf("%s : %d\n",ipstr,port);
		perror("send to");
		exit(1);
	}

}

void shut_down(){
	free(td->n);
	free(td);
	printf("\n pair arreté !\n");
	exit(0);
}
		
int main(){
	setlocale(LC_ALL, "");
	signal(SIGINT,shut_down);
	signal(SIGHUP,shut_down);
	t_table_voisin t[15];
	init_tab(t);
	td=calloc(1, sizeof(*td));
	td->len=1;
	td->n=calloc(1,sizeof(Node));
	init(&td->n[0]);
	memcpy(td->n[0].data,"test de l'insomnie", strlen("test de l'insomnie"));
	td->id=td->n[0].id;
	server(t,td);
}




/*Lorsqu’un nœud reçoit un TLVNeighbour, il envoie un TLVNetwork Hashà l’adresse contenuedans le TLVNeighbour(mais n’ajoute pas le nouveau voisin à sa table de voisins — il ne le feraque lorsqu’il recevra un paquet correct de la part de ce dernier). */
