#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "../serveur/network.h"

int main() {
    char pseudo[64], input[256], buffer[512];

    // Désactive le buffering stdout pour Docker/Alpine
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("Tentative de connexion au serveur...\n");

    int sock = connect_to_server("serveur", 5000);
    printf("Connexion réussie !\n");

    // --- Pseudo ---
    receive_line(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);  // "Pseudo ?"

    fgets(pseudo, sizeof(pseudo), stdin);
    pseudo[strcspn(pseudo, "\n")] = 0;
    send_line(sock, pseudo);

    // --- Message de bienvenue ---
    receive_line(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);

    // --- Boucle interactive ---
    while (1) {
        // Prompt côté client
        printf("Commande > \n");
        fflush(stdout);

        // Lecture utilisateur
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;
        if (strcmp(input, "QUIT") == 0) break;

        // Envoi commande
        send_line(sock, input);

        // Réception résultat
        receive_line(sock, buffer, sizeof(buffer));
        printf("%s\n", buffer);
    }

    close_socket(sock);
    return 0;
}
