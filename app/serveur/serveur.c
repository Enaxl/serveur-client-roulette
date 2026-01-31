#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "network.h"
#include "../manager/player_manager.h"
#include "../games/roulette.h"

void* handle_client(void* arg) {
    int client_sock = *((int*)arg);
    free(arg);
    char buffer[256];

    // 1. Accueil et Pseudo
    send_line(client_sock, "Bienvenue au Casino ! Quel est votre pseudo ?");
    if (receive_line(client_sock, buffer, sizeof(buffer)) <= 0) {
        close(client_sock);
        return NULL;
    }
    buffer[strcspn(buffer, "\n")] = 0;

    Player *p = create_player_for_socket(client_sock, buffer);
    if (!p) {
        send_line(client_sock, "Serveur plein.");
        close(client_sock);
        return NULL;
    }

    // 2. Boucle du Lobby (Menu Principal)
    while (1) {
        send_line(client_sock, "\n=== MENU CASINO ===");
        send_line(client_sock, "1. Roulette (Salle d'attente)");
        send_line(client_sock, "2. Quitter");
        send_line(client_sock, "END_MSG");

        if (receive_line(client_sock, buffer, sizeof(buffer)) <= 0) break;

        // --- SECTION ROULETTE (C'est ICI que tu mets ton bloc) ---
        if (strncmp(buffer, "1", 1) == 0) {
            handle_roulette_waiting(p);

            // Boucle interne : Le joueur est dans la salle
            while (1) {
                if (receive_line(client_sock, buffer, sizeof(buffer)) <= 0) {
                    leave_roulette(p);
                    goto cleanup; // Sortie forcée si déconnexion
                }

                if (strncmp(buffer, "MISE", 4) == 0) {
                    int amount; char type[32], val[32];
                    if (sscanf(buffer, "MISE %d %s %s", &amount, type, val) == 3) {
                        if (check_coins(p, amount)) {
                            remove_coins(p, amount);
                            place_roulette_bet(p, amount, type, val);
                            send_line(client_sock, "[OK] Mise enregistree ! Attente du tirage...\nEND_MSG");
                        } else {
                            send_line(client_sock, "[ERREUR] Coins insuffisants !\nEND_MSG");
                        }
                    }
                }
                else if (buffer[0] == '1') {
                    leave_roulette(p);
                    send_line(client_sock, "[INFO] Retour au menu principal...\nEND_MSG");
                    break; // Sort de la boucle interne -> revient au menu casino
                }
            }
        }
        else if (buffer[0] == '2') {
            break; // Sort de la boucle principale -> déconnexion
        }
    }

cleanup:
    remove_player_by_socket(client_sock);
    close(client_sock);
    return NULL;
}

int main() {
    int server_sock = create_server(5000);
    init_player_manager();

    // Lancement du thread qui gère le temps et les tirages de la roulette
    pthread_t roulette_thread;
    pthread_create(&roulette_thread, NULL, roulette_timer_thread, NULL);

    printf("Casino ouvert sur le port 5000 (Mode Multi-Clients)...\n");

    while (1) {
        int client_sock = accept_client(server_sock);
        pthread_t t;
        int* sock_ptr = malloc(sizeof(int));
        *sock_ptr = client_sock;
        pthread_create(&t, NULL, handle_client, sock_ptr);
        pthread_detach(t);
    }
    return 0;
}