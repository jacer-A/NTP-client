#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include<sys/socket.h>
#include<netdb.h>
#include <unistd.h>

#include "../lib/ntpsocket.h"


void translate_socket(char *DNSname, char *port, struct addrinfo **p_servinfo) {

    struct addrinfo hints;
    memset(&hints,0,sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol= IPPROTO_UDP;
    if(getaddrinfo(DNSname, port, &hints, p_servinfo) !=0) {
        fprintf(stderr, "%s\n", "getaddrinfo() failed!");
        exit(1);
    }
}

int connect_to(struct addrinfo *servinfo) {

    struct addrinfo *p;
    int sock;
    for(p = servinfo; p != NULL; p = p->ai_next) {
        // create socket:
        if ((sock = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            continue;
        }

        // establish connection:
        if (connect(sock, p->ai_addr, p->ai_addrlen) != 0) {
            close(sock);
            fprintf(stderr, "%s\n", "connect() failed!");
            continue;
        }
        break;
    }

    if (p==NULL) {
        fprintf(stderr, "ntpclient failed to connect any socket\n");
        exit(1);
    }
    // => client socket created and connected !

    return sock;
}

void send_pack(int sock, uint8_t *pack, size_t pack_len) {

    long nsent = 0;
    while(nsent != pack_len){
        int bytes = send(sock, pack + nsent, pack_len - nsent, 0);
        if (bytes < 0){
            fprintf(stderr, "Sending data failed!\n");
            break;
        }
        nsent += bytes;
    }
}

void receive_pack(int sock, uint8_t *pack) {

    size_t nreceived = 0;
    while (nreceived != 48) {
        int nbytes = 0;
        uint8_t buffer[48];
        nbytes = recv(sock, buffer, 48, 0);

        if (nbytes == 0) {
            break;
        }

        memcpy(pack + nreceived, buffer, nbytes);
        nreceived += nbytes;
    }
}


