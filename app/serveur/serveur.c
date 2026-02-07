#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "network.h"
#include "../manager/player_manager.h"
#include "../games/roulette.h"
#include "../games/blackjack.h"

void* handle_client(void* arg) {
    int client_sock = *((int*)arg);
    free(arg);
    char buffer[256];

    // Accueil
    send_line(client_sock, "Bienvenue au Casino ! Quel est votre pseudo ?");
    if (receive_line(client_sock, buffer, sizeof(buffer)) <= 0) {
        close(client_sock);
        return NULL;
    }
    buffer[strcspn(buffer, "\r\n")] = 0;

    Player *p = create_player_for_socket(client_sock, buffer);
    if (!p) {
        send_line(client_sock, "Serveur plein.");
        close(client_sock);
        return NULL;
    }

    char welcome_msg[256];
    // Affichage solde + pseudo
    snprintf(welcome_msg, sizeof(welcome_msg),
             "\n[INFO] Votre pseudo est : %s\n[INFO] Vous avez un solde de : %d coins\n",
             p->pseudo, p->total_coins);
    send_line(client_sock, welcome_msg);

    while (1) {
        send_line(client_sock, "\n=== MENU CASINO ===");
        send_line(client_sock, "1. Roulette");
        send_line(client_sock, "2. BlackJack");
        send_line(client_sock, "3. Quitter");
        send_line(client_sock, "END_MSG");

        if (receive_line(client_sock, buffer, sizeof(buffer)) <= 0) break;
        buffer[strcspn(buffer, "\r\n")] = 0; // Nettoyage car on a eu de nombreux bugs

        if (buffer[0] == '1') {
            // Gestion de la roulette côté serveur
            handle_roulette_waiting(p);
            while (1) {
                if (receive_line(client_sock, buffer, sizeof(buffer)) <= 0) {
                    leave_roulette(p); goto cleanup;
                }
                buffer[strcspn(buffer, "\r\n")] = 0;

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
                    break;
                }
            }
        }
        else if (buffer[0] == '2') {
            // Gestion du blackjack côté serveur
            handle_blackjack_waiting(p);
            while (1) {
                if (receive_line(client_sock, buffer, sizeof(buffer)) <= 0) {
                    leave_blackjack(p); goto cleanup;
                }
                buffer[strcspn(buffer, "\r\n")] = 0;

                if (strncmp(buffer, "MISE", 4) == 0 ||
                    strcmp(buffer, "HIT") == 0 ||
                    strcmp(buffer, "STAND") == 0) {
                    process_bj_command(p, buffer);
                }
                else if (buffer[0] == '1') {
                    leave_blackjack(p);
                    break;
                }
            }
        }
        else if (buffer[0] == '3') {
            break;
        }
    }

cleanup:
    printf("[DISCONNECT] %s a quitté le casino.\n", p->pseudo);
    remove_player_by_socket(client_sock);
    close(client_sock);
    return NULL;
}

int main() {
    int server_sock = create_server(5000);
    init_player_manager();

    pthread_t roulette_thread, bj_thread;
    pthread_create(&roulette_thread, NULL, roulette_timer_thread, NULL);
    pthread_create(&bj_thread, NULL, blackjack_timer_thread, NULL);

    printf(" CASINO SERVER - PORT 5000 - ACTIVE\n");

    while (1) {
        int client_sock = accept_client(server_sock);
        if (client_sock < 0) continue;
        pthread_t t;
        int* sock_ptr = malloc(sizeof(int));
        *sock_ptr = client_sock;
        pthread_create(&t, NULL, handle_client, sock_ptr);
        pthread_detach(t);
    }
    return 0;
}