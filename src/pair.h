#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <openssl/sha.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include "tlv.h"
#include "protocole.h"
#include <locale.h>
#include "struct_data.h"


/* explication poll pour mon binome:

struct pollfd {
    int   fd;         //Descripteur de fichier 
    short events;     //Événements attendus    
    short revents;    // Événements détectés    
};
*/
void client(struct in6_addr addr,uint16_t port, int s,unsigned char *buff, int len);

