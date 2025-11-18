#include <stdio.h>
#include "network.h"

void handle_client(int client_sock) {
    char buffer[256];
    int n = receive_message(client_sock, buffer, sizeof(buffer));
    if (n <= 0) return;

    printf("Message reçu : %s\n", buffer);

    buffer[0] = 'R';
    if (n >= sizeof(buffer) - 2) n = sizeof(buffer) - 3;
    buffer[n] = '#';
    buffer[n + 1] = '\0';

    printf("Message après traitement : %s\n", buffer);
    send_message(client_sock, buffer);
}

int main() {
    int server_sock = create_server(5000);
    printf("Serveur en écoute sur le port 5000...\n");

    while (1) {
        int client_sock = accept_client(server_sock);
        handle_client(client_sock);
        close_socket(client_sock);
    }

    close_socket(server_sock);
    return 0;
}
