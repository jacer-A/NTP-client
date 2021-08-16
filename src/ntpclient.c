#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include "math.h"

#include "../lib/ntpsocket.h"


struct data {
    double Offset;
    double Delay;
    double Dispersion;
    float root_disp;
};

void take_measure(int sock, struct data *measure) {

    struct timespec tp;

    // prepare packet:
    u_int8_t request[48];
    memset(request, 0, 48);
    request[0]= 4 << 3 | 3 << 0;

    // send packet:
    clock_gettime(CLOCK_REALTIME, &tp);
    send_pack(sock, request, 48);
    double T1= tp.tv_sec + tp.tv_nsec / pow(10,9);

    // receive packet:
    u_int8_t response[48];
    receive_pack(sock, response);
    clock_gettime(CLOCK_REALTIME, &tp);
    double T4= tp.tv_sec + tp.tv_nsec / pow(10,9);

    // handle packet:
    double T2= ntohl( ((uint32_t*)response)[8] ) - 2208988800 +
               ntohl( ((uint32_t*)response)[9] ) / pow(2,32);
    double T3= ntohl( ((uint32_t*)response)[10] ) - 2208988800 +
               ntohl( ((uint32_t*)response)[11] ) / pow(2,32);

    measure->root_disp= ntohs( ((uint16_t*)response)[4] ) +
                        ntohs( ((uint16_t*)response)[5] ) / pow(2,16);

    measure->Offset = 0.5*( (T2-T1)+(T3-T4) );
    measure->Delay = 0.5*( (T4-T1)-(T3-T2) );
}

void calcul_Dispersion(struct data *serv_stats[], int request_number) {

    double max = serv_stats[request_number]->Delay;
    double min = serv_stats[request_number]->Delay;
    for (int i=request_number-1; i >= 0 && i > request_number-8; i--){
        if (serv_stats[i]->Delay > max){
            max= serv_stats[i]->Delay;
        }
        if (serv_stats[i]->Delay < min){
            min= serv_stats[i]->Delay;
        }
    }
    serv_stats[request_number]->Dispersion= 2*(max - min);
}



int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Not enough arguments\nTry:\n./ntpclient <number of requests> <server1> <server2> ...\n");
        exit(1);
    }

    int n= atoi(argv[1]);
    if (n==0) {
        fprintf(stderr, "Number of requests must be greater than 0\n");
        exit(1);
    }

    // saving server info to avoid calling getaddrinfo() more than once:
    char *port= "123";
    struct addrinfo *servinfo[argc-2];
    for (int i=0; i<argc-2; i++) {
        translate_socket(argv[i+2], port, &servinfo[i]);
    }

    // collecting data:
    struct data *stats[argc-2][n];
    for (int request_nr=0; request_nr < n; request_nr++) {
        for (int serv_nr=0; serv_nr < argc-2; serv_nr++) {

            int sock = connect_to(servinfo[serv_nr]);

            stats[serv_nr][request_nr]= malloc(sizeof(struct data));

            take_measure(sock, stats[serv_nr][request_nr]);

            calcul_Dispersion(stats[serv_nr], request_nr);

            printf("%s;%d;%lf;%lf;%lf;%lf\n", argv[serv_nr+2], request_nr,
                   stats[serv_nr][request_nr]->root_disp, stats[serv_nr][request_nr]->Dispersion,
                    stats[serv_nr][request_nr]->Delay, stats[serv_nr][request_nr]->Offset);

            close(sock);

        }
        if(request_nr<n-1) {sleep(8);}
    }

/*
    // saving data in text files:
    for (int i=0; i < argc-2; i++) {
        char serv_name[50]="datas/";    strcpy(serv_name, argv[i+2]);
        strcat(serv_name, ".txt");
        FILE *serv_stats= fopen(serv_name, "w");

        fprintf(serv_stats, "N\tDelay\tDispersion\tOffset\n");
        for (int j=0; j<n; j++) {
            fprintf(serv_stats, "%d\t%lf\t%lf\t%lf\n", j, stats[i][j]->Delay,
                    stats[i][j]->Dispersion, stats[i][j]->Offset);
        }
        fclose(serv_stats);
    }
*/

    // cleaning:
    for (int i=0; i<argc-2; i++) {
        freeaddrinfo(servinfo[i]);
        for (int j=0; j<n; j++) {
            free(stats[i][j]);
        }
    }

    return 0;
}

