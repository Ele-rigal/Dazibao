#pragma once

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <netinet/in.h>
#include "struct_data.h"


typedef struct Pad1 { 
	uint8_t type;
}__attribute__((packed))t_Pad1;

typedef struct PadN {
	uint8_t Type;
	uint8_t Length;
	uint8_t * MBZ;
}__attribute__((packed)) t_PadN;		

typedef struct Neighbour_request{
	uint8_t Type;
	uint8_t Length;
}__attribute__((packed)) t_Neighbour_request;

typedef struct Neighbour{
	uint8_t Type;
	uint8_t Length;
	struct in6_addr ip;
	uint16_t Port;
}__attribute__((packed)) t_Neighbour;

typedef struct Network_hash{
	uint8_t Type;
	uint8_t Length;
	unsigned char NetworkHash[16];
}__attribute__((packed)) t_Network_hash;

typedef struct Network_State_Request{
	uint8_t Type;
	uint8_t Length;
}__attribute__((packed)) t_Network_State_Request;

typedef struct Node_hash{
	uint8_t Type;
	uint8_t Length;
	uint64_t node_id;
	uint16_t Seqno;
	unsigned char node_h[16];
}__attribute__((packed)) t_Node_hash;

typedef struct Node_State_Request{
	uint8_t Type;
	uint8_t Length;
	uint64_t node_id;
} __attribute__((packed)) t_Node_State_Request;

typedef struct Node_state{
	uint8_t Type;
	uint8_t Length;
	uint64_t node_id;
	uint16_t Seqno;
	unsigned char Node_Hash[16];
	char Data[192];
}__attribute__((packed)) t_Node_state;

typedef struct Warning{
	uint8_t Type;
	uint8_t Length;
	char Message[1024];
}__attribute__((packed)) t_Warning;

typedef struct base_TLV{
	uint8_t Type;
	uint8_t Length;
}__attribute__((packed)) base_TLV;

typedef union TLV{
	t_Pad1 pa1;
	t_PadN padn;
	t_Neighbour_request voisin_req;
	t_Neighbour voisin;
	t_Network_hash networkh;
	t_Network_State_Request network_statereq;
	t_Node_hash nodeh;
	t_Node_State_Request node_statereq;
	t_Node_state node_state;
	t_Warning warning;
	base_TLV tlv;
}__attribute__((packed)) t_TLV;
		
typedef struct datagramme_udp{
	uint8_t Magic;
	uint8_t Version;
	uint16_t Body_length;
    char body[1020];
	
}__attribute__((packed)) datagramme;


