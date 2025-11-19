#include <stdio.h>
#include <string.h>
#include <time.h>

#include "network.h"
#include "../games/roulette.h"
#include "../entity/player.h"
#include "../manager/player_manager.h"

void handle_client(int client_sock) {
    char buffer[256];

    // --- Demande du pseudo ---
    send_message(client_sock, "Pseudo ? ");
    int n = receive_message(client_sock, buffer, sizeof(buffer));
    if (n <= 0) return;
    buffer[strcspn(buffer, "\n")] = 0;

    // --- Création du joueur ---
    Player *p = create_player_for_socket(client_sock, buffer);
    if (!p) {
        send_message(client_sock, "Erreur : impossible de créer le joueur.\n");
        return;
    }

    printf("Nouveau joueur connecté : %s (socket %d)\n", p->pseudo, client_sock);
    send_message(client_sock, "Bienvenue ! Exemple : GAME roulette MISE 10 number 17\n");

    // --- Boucle commandes ---
    while (1) {
        n = receive_message(client_sock, buffer, sizeof(buffer));
        if (n <= 0) {
            printf("Déconnexion : %s\n", p->pseudo);
            return;
        }

        buffer[strcspn(buffer, "\n")] = 0; // nettoyage
        if (strlen(buffer) == 0) continue;

        if (p->total_coins == 0) {
            send_message(client_sock, "Vous n'avez plus de jetons, déconnexion.\n");
            printf("Joueur %s out of coins.\n", p->pseudo);
            return;
        }

        printf("[%s] commande reçue : %s\n", p->pseudo, buffer);

        char game[32];
        char command[224];
        if (sscanf(buffer, "GAME %31s %223[^\n]", game, command) < 1) {
            send_message(client_sock, "Format invalide.\n");
            continue;
        }

        if (strcmp(game, "roulette") == 0) {
            handle_roulette(p, command);
        } else {
            send_message(client_sock, "Jeu inconnu.\n");
        }
    }
}

int main() {
    int server_sock = create_server(5000);
    printf("Serveur en écoute sur le port 5000...\n");

    init_player_manager();

    // --- Boucle serveur multi-joueurs (bloquante simple) ---
    while (1) {
        int client_sock = accept_client(server_sock);
        handle_client(client_sock);
        close_socket(client_sock);
    }

    close_socket(server_sock);
    return 0;
}
