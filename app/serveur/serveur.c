#include <stdio.h>
#include <string.h>
#include "../serveur/network.h"
#include "../games/roulette.h"
#include "../entity/player.h"
#include "../manager/player_manager.h"

void handle_client(int client_sock) {
    char buffer[256];

    send_line(client_sock, "Pseudo ?");
    int n = receive_line(client_sock, buffer, sizeof(buffer));
    if (n <= 0) return;

    Player *p = create_player_for_socket(client_sock, buffer);
    if (!p) {
        send_line(client_sock, "Erreur : impossible de créer le joueur.");
        return;
    }

    printf("Nouveau joueur connecté : %s\n", p->pseudo);
    send_line(client_sock, "Bienvenue ! Exemple : GAME roulette MISE 10 number 17");

    while (1) {
        n = receive_line(client_sock, buffer, sizeof(buffer));
        if (n <= 0) {
            printf("Déconnexion : %s\n", p->pseudo);
            return;
        }

        if (p->total_coins == 0) {
            send_line(client_sock, "Vous n'avez plus de jetons, déconnexion.");
            return;
        }

        printf("[%s] commande reçue : %s\n", p->pseudo, buffer);

        char game[32], command[224];
        if (sscanf(buffer, "GAME %31s %223[^\n]", game, command) != 2) {
            send_line(client_sock, "Format invalide.");
            continue;
        }

        if (strcmp(game, "roulette") == 0) {
            handle_roulette(p, command);
        } else {
            send_line(client_sock, "Jeu inconnu.");
        }
    }
}



int main() {
    int server_sock = create_server(5000);
    printf("Serveur en écoute sur le port 5000...");

    init_player_manager();

    while (1) {
        int client_sock = accept_client(server_sock);
        handle_client(client_sock);
        close_socket(client_sock);
    }

    close_socket(server_sock);
    return 0;
}
