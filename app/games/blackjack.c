#include "blackjack.h"
#include "../serveur/network.h"
#include "../manager/player_manager.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_BJ_PLAYERS 10 // Nb de joueurs maximum
#define WAIT_BJ_START 20 // Attente avant que le tour se lance
#define WAIT_BJ_BET 15 // Attente pour la mise des joueurs
#define WAIT_BJ_TURN 15 // Attente pour HIT et STAND
#define WAIT_TRANSITION 10 // Transtion entre 2 tours

static Player* bj_players[MAX_BJ_PLAYERS];
static int bj_nb = 0; // Joueurs en game
static pthread_mutex_t bj_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t bj_cond = PTHREAD_COND_INITIALIZER;
static int bj_is_running = 0; // Savoir si une partie est en cours ou non

typedef struct {
    int score;
    int bet;
    int playing;
} BJState;

static BJState player_states[MAX_BJ_PLAYERS];

// Logique des cartes
int draw_card() {
    int card = (rand() % 13) + 1;
    if (card > 10) return 10;
    if (card == 1) return 11;
    return card;
}

// Gestion des entrées / sorties de la salle d'attente
void handle_blackjack_waiting(Player *p) {
    pthread_mutex_lock(&bj_mtx);
    if (bj_nb < MAX_BJ_PLAYERS && !bj_is_running) {
        bj_players[bj_nb] = p;
        player_states[bj_nb].bet = 0;
        player_states[bj_nb].playing = 0;
        player_states[bj_nb].score = 0;
        bj_nb++;

        char msg[256];
        snprintf(msg, sizeof(msg), "\n[BJ] Salle BlackJack (%d/%d). Attente d'adversaires...\nEND_MSG", bj_nb, MAX_BJ_PLAYERS);
        send_message(p->client_socket, msg);
        if (bj_nb >= 2) pthread_cond_signal(&bj_cond);
    } else {
        send_line(p->client_socket, "\n[BJ] Salle pleine ou tour en cours. Patientez...\nEND_MSG");
    }
    pthread_mutex_unlock(&bj_mtx);
}

// Partir en fin de partie
void leave_blackjack(Player *p) {
    pthread_mutex_lock(&bj_mtx);
    if (bj_is_running) {
        send_line(p->client_socket, "\n[REFUS] Vous ne pouvez pas quitter pendant une main !\nEND_MSG");
    } else {
        for (int i = 0; i < bj_nb; i++) {
            if (bj_players[i]->client_socket == p->client_socket) {
                for (int j = i; j < bj_nb - 1; j++) {
                    bj_players[j] = bj_players[j+1];
                    player_states[j] = player_states[j+1];
                }
                bj_nb--;
                send_line(p->client_socket, "[INFO] Retour au menu principal...\nEND_MSG");
                break;
            }
        }
    }
    pthread_mutex_unlock(&bj_mtx);
}

// Gestion des commandes faites par le joueur HIT / STAND / MISE
void process_bj_command(Player *p, const char* cmd) {
    pthread_mutex_lock(&bj_mtx);
    if (!bj_is_running) { pthread_mutex_unlock(&bj_mtx); return; }

    for (int i = 0; i < bj_nb; i++) {
        if (bj_players[i]->client_socket == p->client_socket) {
            char buffer_msg[256];
            // MISE
            if (strncmp(cmd, "MISE", 4) == 0 && player_states[i].bet == 0) {
                int amount = 0;
                if (sscanf(cmd, "MISE %d", &amount) == 1) {
                    if (amount > 0 && check_coins(p, amount)) {
                        remove_coins(p, amount);
                        player_states[i].bet = amount;
                        snprintf(buffer_msg, sizeof(buffer_msg), "[BJ] Mise de %d acceptee. Bonne chance !\nEND_MSG", amount);
                        send_line(p->client_socket, buffer_msg);
                    } else {
                        send_line(p->client_socket, "[BJ] Erreur : Solde insuffisant ou deja mise.\nEND_MSG");
                    }
                }
            }
            // HIT / STAND
            else if (player_states[i].playing) {
                if (strcmp(cmd, "HIT") == 0) {
                    int card = draw_card();
                    player_states[i].score += card;
                    if (player_states[i].score > 21) {
                        player_states[i].playing = 0;
                        snprintf(buffer_msg, sizeof(buffer_msg), "[BJ] CARTE : %d. SCORE : %d -> BUST (Perdu) !\nEND_MSG", card, player_states[i].score);
                    } else {
                        snprintf(buffer_msg, sizeof(buffer_msg), "[BJ] CARTE : %d. SCORE : %d (Vite ! HIT/STAND ?)\nEND_MSG", card, player_states[i].score);
                    }
                    send_line(p->client_socket, buffer_msg);
                } else if (strcmp(cmd, "STAND") == 0) {
                    player_states[i].playing = 0;
                    snprintf(buffer_msg, sizeof(buffer_msg), "[BJ] Vous restez sur %d. Attente du croupier...\nEND_MSG", player_states[i].score);
                    send_line(p->client_socket, buffer_msg);
                }
            }
            break;
        }
    }
    pthread_mutex_unlock(&bj_mtx);
}

// Cycle du jeu
void* blackjack_timer_thread(void* arg) {
    (void)arg; srand(time(NULL));
    while(1) {
        pthread_mutex_lock(&bj_mtx);
        // Attente si moins de 2 joueurs
        while (bj_nb < 2) {
            bj_is_running = 0;
            pthread_cond_wait(&bj_cond, &bj_mtx);
        }

        // Mise des joueurs
        bj_is_running = 1;
        for(int i=0; i<bj_nb; i++) {
            player_states[i].bet = 0;
            player_states[i].score = 0;
            player_states[i].playing = 0;
            send_message(bj_players[i]->client_socket, "\n[BJ] DEBUT ! Misez avec : MISE <montant> (20s)\nEND_MSG");
        }
        pthread_mutex_unlock(&bj_mtx);
        sleep(WAIT_BJ_BET);

        // Distribution des 2 cartes
        pthread_mutex_lock(&bj_mtx);
        for(int i=0; i<bj_nb; i++) {
            if (player_states[i].bet == 0) {
                player_states[i].bet = 10; // Si le joueur ne mise pas alors on force une mise minimum
                remove_coins(bj_players[i], 10);
            }
            player_states[i].score = draw_card() + draw_card();
            player_states[i].playing = 1;
            char msg[256];
            snprintf(msg, sizeof(msg), "\n[BJ] Cartes servies. Votre score : %d. (10s pour HIT ou STAND)\nEND_MSG", player_states[i].score);
            send_message(bj_players[i]->client_socket, msg);
        }
        pthread_mutex_unlock(&bj_mtx);
        sleep(WAIT_BJ_TURN);

        // résultats
        pthread_mutex_lock(&bj_mtx);
        int dealer = draw_card() + draw_card();
        while(dealer < 17) dealer += draw_card();

        for(int i=0; i<bj_nb; i++) {
            Player *p = bj_players[i];
            int p_score = player_states[i].score;
            int gain = 0;
            char res_txt[32];

            if (p_score > 21) { strcpy(res_txt, "BUST (Perdu)"); }
            else if (dealer > 21 || p_score > dealer) {
                strcpy(res_txt, "GAGNE !");
                gain = player_states[i].bet * 2;
                add_coins(p, gain);
            } else if (p_score == dealer) {
                strcpy(res_txt, "EGALITE");
                gain = player_states[i].bet;
                add_coins(p, gain);
            } else { strcpy(res_txt, "PERDU"); }

            char out[1024];
            snprintf(out, sizeof(out),
                "\n======================================\n"
                " BLACKJACK - RESULTATS\n"
                "======================================\n"
                " Score Croupier : %d\n"
                " Votre Score    : %d\n"
                " Resultat       : %s\n"
                " Gain           : %d coins\n"
                " Nouveau solde  : %d coins\n"
                "======================================\n"
                "Fin du tour. Tapez '1' pour quitter maintenant.\nEND_MSG",
                dealer, p_score, res_txt, gain, p->total_coins);
            send_message(p->client_socket, out);
        }

        // transition d'une partie a l'autre
        bj_is_running = 0;
        pthread_mutex_unlock(&bj_mtx);

        sleep(WAIT_TRANSITION);

        pthread_mutex_lock(&bj_mtx);
        if (bj_nb > 0) {
            for(int i = 0; i < bj_nb; i++) {
                send_message(bj_players[i]->client_socket, "\n[INFO] Preparation du prochain tour... Restez en ligne ! Prochaine manche dans 10secondes si il y a minimum 2 joueurs\nEND_MSG");
            }
        }
        pthread_mutex_unlock(&bj_mtx);
    }
    return NULL;
}