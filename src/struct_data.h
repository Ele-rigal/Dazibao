#pragma once

#include <stdbool.h>
#include <stdint.h>

uint16_t num_sequence(uint16_t s,int n);

int relat_ternaire(uint16_t s1,uint16_t s2);

uint64_t gen_id();
	
typedef struct Node{
	uint64_t id;
	uint16_t s;
	char data[193];
}__attribute__((packed)) Node;

typedef struct table_donnee{
	Node *n;
	int len;
	uint64_t id;
}t_table_donnee;

void print_buff(void *buff, int len);

unsigned char* h16b(Node *n);

char *h16 (Node *n);

void init(Node *n);

void write_file(t_table_donnee *td);

int partition(Node *tab,int begin,int end,int pivot);

void quick_sort(Node *tab,int begin,int end);

unsigned char *networkh(Node *n,int len);

void comp_seq(t_table_donnee *td,int i,uint16_t Seqno);

void maj_tab_node(Node node,t_table_donnee *td);

void change_donnee_perso(t_table_donnee *t);
