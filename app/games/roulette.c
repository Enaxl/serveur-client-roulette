#include "roulette.h"
#include "../serveur/network.h"
#include "../manager/player_manager.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define WAIT_TIME_BETS 30
#define WAIT_TRANSITION 10

static Player* waiting_players[MAX_PLAYERS];
static int nb_waiting = 0;
static pthread_mutex_t roulette_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_start = PTHREAD_COND_INITIALIZER;

typedef struct {
    int amount;
    char type[32];
    char value[32];
    int active;
} CurrentBet;

static CurrentBet current_bets[MAX_PLAYERS];

// --- Logique du tirage ---
const char *get_color(int number) {
    if (number == 0) return "vert";
    return (number % 2 == 0) ? "noir" : "rouge";
}

const char *get_parity(int number) {
    if (number == 0) return "zéro";
    return (number % 2 == 0) ? "pair" : "impair";
}

RouletteResult spin_roulette() {
    static int initialized = 0;
    if (!initialized) { srand(time(NULL)); initialized = 1; }
    RouletteResult result;
    result.number = rand() % 37;
    result.color = get_color(result.number);
    result.parity = get_parity(result.number);
    return result;
}

// --- Gestion des Entrées / Sorties de salle ---

void handle_roulette_waiting(Player *p) {
    pthread_mutex_lock(&roulette_mtx);
    if (nb_waiting < MAX_PLAYERS) {
        waiting_players[nb_waiting] = p;
        current_bets[nb_waiting].active = 0;
        nb_waiting++;

        char msg[256];
        snprintf(msg, sizeof(msg), "\n[SALLE] Joueurs : %d/2. Attente d'adversaire...\nEND_MSG", nb_waiting);
        send_message(p->client_socket, msg);

        if (nb_waiting >= 2) pthread_cond_signal(&cond_start);
    }
    pthread_mutex_unlock(&roulette_mtx);
}

void leave_roulette(Player *p) {
    pthread_mutex_lock(&roulette_mtx);
    for (int i = 0; i < nb_waiting; i++) {
        if (waiting_players[i]->client_socket == p->client_socket) {
            // On décale les joueurs restants
            for (int j = i; j < nb_waiting - 1; j++) {
                waiting_players[j] = waiting_players[j+1];
                current_bets[j] = current_bets[j+1];
            }
            nb_waiting--;
            break;
        }
    }
    pthread_mutex_unlock(&roulette_mtx);
}

void place_roulette_bet(Player *p, int amount, const char* type, const char* value) {
    pthread_mutex_lock(&roulette_mtx);
    for (int i = 0; i < nb_waiting; i++) {
        if (waiting_players[i]->client_socket == p->client_socket) {
            current_bets[i].amount = amount;
            strncpy(current_bets[i].type, type, 31);
            strncpy(current_bets[i].value, value, 31);
            current_bets[i].active = 1;
            break;
        }
    }
    pthread_mutex_unlock(&roulette_mtx);
}

void* roulette_timer_thread(void* arg) {
    (void)arg;
    while(1) {
        pthread_mutex_lock(&roulette_mtx);
        while (nb_waiting < 2) {
            pthread_cond_wait(&cond_start, &roulette_mtx);
        }

        for (int i = 0; i < nb_waiting; i++) {
            current_bets[i].active = 0;
            char start_msg[512];
            snprintf(start_msg, sizeof(start_msg),
                "\n[JEU] La partie commence ! Vous avez 30 secondes.\n"
                "[INFO] Commandes possibles :\n"
                " - MISE <montant> color <rouge/noir>\n"
                " - MISE <montant> number <0-36>\n"
                " - MISE <montant> parity <pair/impair>\n"
                "Tapez '1' pour quitter la salle à tout moment.\nEND_MSG");
            send_message(waiting_players[i]->client_socket, start_msg);
        }
        pthread_mutex_unlock(&roulette_mtx);

        for(int t=20; t>=0; t-=10) {
            sleep(10);
            pthread_mutex_lock(&roulette_mtx);
            for(int i=0; i<nb_waiting; i++) {
                char timer_msg[64];
                snprintf(timer_msg, sizeof(timer_msg), "[TIMER] Plus que %d secondes pour miser...\nEND_MSG", t);
                send_message(waiting_players[i]->client_socket, timer_msg);
            }
            pthread_mutex_unlock(&roulette_mtx);
        }

        pthread_mutex_lock(&roulette_mtx);
        if (nb_waiting > 0) {
            RouletteResult res = spin_roulette();
            for (int i = 0; i < nb_waiting; i++) {
                Player *p = waiting_players[i];
                int gain = 0;
                if (current_bets[i].active) {
                    if (strcmp(current_bets[i].type, "number") == 0 && atoi(current_bets[i].value) == res.number) gain = current_bets[i].amount * 35;
                    else if (strcmp(current_bets[i].type, "color") == 0 && strcmp(current_bets[i].value, res.color) == 0) gain = current_bets[i].amount * 2;
                    else if (strcmp(current_bets[i].type, "parity") == 0 && strcmp(current_bets[i].value, res.parity) == 0) gain = current_bets[i].amount * 2;
                    if (gain > 0) add_coins(p, gain);
                }

                char out[1024];
                snprintf(out, sizeof(out),
                    "\n======================================\n"
                    " RESULTAT : %d %s %s\n"
                    "======================================\n"
                    "%s Solde : %d coins.\n"
                    "Prochain tour dans 10s. Tapez '1' pour quitter.\nEND_MSG",
                    res.number, res.color, res.parity, (gain>0)?"[GAGNE]":"[PERDU]", p->total_coins);
                send_message(p->client_socket, out);
            }
        }
        pthread_mutex_unlock(&roulette_mtx);

        sleep(10);

        pthread_mutex_lock(&roulette_mtx);
        if (nb_waiting == 1) {
            send_message(waiting_players[0]->client_socket, "\n[INFO] Adversaire parti. En attente d'un nouveau joueur (ou tapez '1')...\nEND_MSG");
        }
        pthread_mutex_unlock(&roulette_mtx);
    }
    return NULL;
}