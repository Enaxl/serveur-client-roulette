#include <stdio.h>
#include "../serveur/network.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <serveur> <message>\n", argv[0]);
        return 1;
    }

    int sock = connect_to_server(argv[1], 5000);
    send_message(sock, argv[2]);

    char buffer[256];
    int n = receive_message(sock, buffer, sizeof(buffer));
    if (n > 0) {
        printf("Réponse du serveur : %s\n", buffer);
    }

    close_socket(sock);
    return 0;
}
