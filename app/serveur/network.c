#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

int create_server(int port) {
    int sock;
    struct sockaddr_in addr;
    int opt = 1;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); exit(1); }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); exit(1); }

    if (listen(sock, 5) < 0) { perror("listen"); exit(1); }

    return sock;
}

int accept_client(int server_sock) {
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &len);
    if (client_sock < 0) { perror("accept"); exit(1); }
    return client_sock;
}

/* -----------------------------------------------
   Version corrigée qui fonctionne sous Alpine
   (remplace gethostbyname par getaddrinfo)
------------------------------------------------- */
int connect_to_server(const char *host, int port) {
    int sock;
    struct addrinfo hints, *res;
    char port_str[6];

    snprintf(port_str, sizeof(port_str), "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;         // IPv4
    hints.ai_socktype = SOCK_STREAM;   // TCP

    if (getaddrinfo(host, port_str, &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(1);
    }

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        perror("socket");
        freeaddrinfo(res);
        exit(1);
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        close(sock);
        freeaddrinfo(res);
        exit(1);
    }

    freeaddrinfo(res);
    return sock;
}

int send_message(int sock, const char *msg) {
    return write(sock, msg, strlen(msg));
}

int receive_message(int sock, char *buffer, int size) {
    int n = read(sock, buffer, size - 1);
    if (n > 0) buffer[n] = '\0';
    return n;
}

void close_socket(int sock) {
    close(sock);
}
