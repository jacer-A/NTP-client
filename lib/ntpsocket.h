#ifndef BLOCK5_NTPSOCKET_H
#define BLOCK5_NTPSOCKET_H


void translate_socket(char *DNSname, char *port, struct addrinfo **p_servinfo);
int connect_to(struct addrinfo *servinfo);
void send_pack(int sock, uint8_t *pack, size_t pack_len);
void receive_pack(int sock, uint8_t *pack);



#endif //BLOCK5_NTPSOCKET_H
