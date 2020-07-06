#pragma once

#include "tlv.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdint.h>
#include "struct_data.h"

#define TRUE 1
#define FALSE 0
#define DOMAINE AF_INET6
#define PORT 1212
#define ADD4 "::ffff:81.194.27.155"
#define ADD6 "2001:660:3301:9200::51c2:1b9b"

typedef union request{
	datagramme data;
	unsigned char buff[1024];
}t_req;

/*typedef struct table_data {
	Node *n;
}t_table_data;*/
/***rappel structures:

typedef struct Neighbour_request{
	uint8_t Type;
	uint8_t Length;
}t_Neighbour_request;

typedef struct Neighbour{
	uint8_t Type;
	uint8_t Length;
	struct in6_addr ip;
	uint16_t Port;
}t_Neighbour;

***/
typedef  struct table_voisin{
	int state;
	struct in6_addr addr;
	char ip[INET6_ADDRSTRLEN];
	uint16_t port;
	time_t lastseen;
}t_table_voisin;

void ask_voisin(struct in6_addr addr,uint16_t port,int s);

//int searchTab(t_table_voisin t[15],char *adresse,int s,int p);

int searchTab(t_table_voisin *t,char *ip);

void majTab_voisin(uint16_t port,struct in6_addr addr,char *ip,time_t lasteen,t_table_voisin *t);

void init_tab(t_table_voisin *t);

void parcours(t_table_voisin *t);

void read_tlv(t_req r,t_table_donnee *td,struct in6_addr addr,int s,uint16_t port,t_table_voisin *t);

int nb_voisin(t_table_voisin *t);

int check_entete(datagramme data);

/*void interprete_type(int t,t_req recu,int i,t_table_donnee *td,struct in6_addr addr,int s,uint16_t port,t_table_voisin *voisin);*/
void interprete_type(int t,t_TLV *tlv,t_table_donnee *td,struct in6_addr addr,int s,uint16_t port,t_table_voisin *voisin,t_req recu);

void check_Network_hash(t_Network_hash nh,t_table_donnee *td, struct in6_addr addr,uint16_t port,int s);

void search_nodh(t_Node_hash nh,t_table_donnee *td,struct in6_addr addr,uint16_t port,int s);

void init_connect(int s);

void send_neth(t_table_donnee *td,struct in6_addr addr,uint16_t port,int s);

void send_warning(struct in6_addr addr,int s,uint16_t port,char *message);
