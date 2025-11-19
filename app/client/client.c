#include <stdio.h>
#include <string.h>
#include "../serveur/network.h"

int main(int argc, char **argv) {
    char pseudo[64];
    char input[256];
    int sock;

    printf("Pseudo ? ");
    fgets(pseudo, sizeof(pseudo), stdin);
    pseudo[strcspn(pseudo, "\n")] = 0;

    sock = connect_to_server("serveur", 5000);
    send_message(sock, pseudo);

    // --- Boucle interactive ---
    while (1) {
        printf("Commande > ");
        if (!fgets(input, sizeof(input), stdin)) break;

        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;

        if (strcmp(input, "QUIT") == 0) break;

        send_message(sock, input);

        char buffer[512];
        int n = receive_message(sock, buffer, sizeof(buffer));
        if (n > 0) {
            printf("%s\n", buffer);
        }
    }

    close_socket(sock);
    return 0;
}
