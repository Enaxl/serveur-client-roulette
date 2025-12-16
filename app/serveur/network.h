#ifndef NETWORK_H
#define NETWORK_H

int create_server(int port);
int accept_client(int server_sock);
int connect_to_server(const char *host, int port);
int send_line(int sock, const char *msg);
int receive_line(int sock, char *buffer, int size);
void close_socket(int sock);

#endif
