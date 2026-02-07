#include "roulette.h"
#include "../serveur/network.h"
#include "../manager/player_manager.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PLAYERS_ROOM 10 
#define WAIT_FOR_START 30   // Attente quand 2 joueurs sont présents
#define WAIT_TIME_BETS 30   // Temps pour miser
#define WAIT_TRANSITION 30  // Temps pour lire le résultat et décider de rester

static Player* waiting_players[MAX_PLAYERS_ROOM];
static int nb_waiting = 0;
static pthread_mutex_t roulette_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_start = PTHREAD_COND_INITIALIZER;

static int is_betting_phase = 0; // État de la phase de jeu pour bloquer la sortie

typedef struct {
    int amount;
    char type[32];
    char value[32];
    int active;
} CurrentBet;

static CurrentBet current_bets[MAX_PLAYERS_ROOM];

// Logique du tirage
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

//Gestion des Entrées / Sorties de salle
void handle_roulette_waiting(Player *p) {
    pthread_mutex_lock(&roulette_mtx);

    // On refuse si salle pleine ou si une partie a déjà commencé (phase de mise)
    if (nb_waiting >= MAX_PLAYERS_ROOM || is_betting_phase) {
        send_line(p->client_socket, "\n[ERREUR] Salle pleine ou partie deja en cours de mise. Attendez le prochain tour.\nEND_MSG");
        pthread_mutex_unlock(&roulette_mtx);
        return;
    }

    waiting_players[nb_waiting] = p;
    current_bets[nb_waiting].active = 0;
    nb_waiting++;

    char msg[256];
    snprintf(msg, sizeof(msg), "\n[SALLE] Vous avez rejoint la salle. Joueurs presents : %d/%d.\nEND_MSG", nb_waiting, MAX_PLAYERS_ROOM);
    send_message(p->client_socket, msg);

    // Si on est au moins 2, on réveille le thread pour lancer le décompte de 30s
    if (nb_waiting >= 2) {
        pthread_cond_signal(&cond_start);
    }

    pthread_mutex_unlock(&roulette_mtx);
}

void leave_roulette(Player *p) {
    pthread_mutex_lock(&roulette_mtx);

    // permet d'interdire un joueur de partir
    if (is_betting_phase) {
        send_line(p->client_socket, "\n[REFUS] Impossible de quitter pendant la phase de mise ! Attendez le resultat.\nEND_MSG");
        pthread_mutex_unlock(&roulette_mtx);
        return;
    }

    for (int i = 0; i < nb_waiting; i++) {
        if (waiting_players[i]->client_socket == p->client_socket) {
            // Décalage des joueurs restants
            for (int j = i; j < nb_waiting - 1; j++) {
                waiting_players[j] = waiting_players[j+1];
                current_bets[j] = current_bets[j+1];
            }
            nb_waiting--;
            send_line(p->client_socket, "[INFO] Retour au menu principal...\nEND_MSG");
            break;
        }
    }
    pthread_mutex_unlock(&roulette_mtx);
}

// Placer sa mise sur la roulette
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

// Cycle de vie du jeu
void* roulette_timer_thread(void* arg) {
    (void)arg;
    while(1) {
        pthread_mutex_lock(&roulette_mtx);

        // 1. attente de joueurs
        while (nb_waiting < 2) {
            is_betting_phase = 0;
            pthread_cond_wait(&cond_start, &roulette_mtx);
        }

        // 2. préparation de 30 secondes pour laisser plusieurs joueurs arrivés
        for (int i = 0; i < nb_waiting; i++) {
            send_message(waiting_players[i]->client_socket,
                "\n[SALLE] 2 joueurs minimum atteints ! La partie debute dans 30s.\n"
                "[INFO] D'autres joueurs peuvent encore rejoindre pendant ce temps.\nEND_MSG");
        }
        pthread_mutex_unlock(&roulette_mtx);

        sleep(WAIT_FOR_START);

        // les joueurs peuvent miser
        pthread_mutex_lock(&roulette_mtx);
        is_betting_phase = 1; // on bloque la sortie

        for (int i = 0; i < nb_waiting; i++) {
            current_bets[i].active = 0;
            char start_msg[512];
            snprintf(start_msg, sizeof(start_msg),
                "\n[JEU] Les paris sont OUVERTS ! (Joueurs en piste : %d)\n"
                "[PARIS] Vous avez 30 secondes pour miser.\n"
                "Commandes : MISE <montant> color <rouge/noir> | number <0-36> | parity <pair/impair>\n"
                "La sortie est bloquee pendant cette phase.\nEND_MSG", nb_waiting);
            send_message(waiting_players[i]->client_socket, start_msg);
        }
        pthread_mutex_unlock(&roulette_mtx);

        sleep(WAIT_TIME_BETS);

        // tirage et affichage du résultat
        pthread_mutex_lock(&roulette_mtx);
        is_betting_phase = 0; // on autorise de nouveau la sortie du joueur de la partie

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
                    "%s Solde actuel : %d coins.\n"
                    "Nouvelle partie dans 30s. Appuyez sur '1' pour quitter.\nEND_MSG",
                    res.number, res.color, res.parity, (gain > 0) ? "[GAGNE]" : "[PERDU]", p->total_coins);
                send_message(p->client_socket, out);
            }
        }
        pthread_mutex_unlock(&roulette_mtx);

        // transition entre 2 tours
        sleep(WAIT_TRANSITION);

        pthread_mutex_lock(&roulette_mtx);
        if (nb_waiting == 1) {
            send_message(waiting_players[0]->client_socket, "\n[INFO] Trop de joueurs sont partis. En attente d'un nouvel adversaire...\nEND_MSG");
        }
        pthread_mutex_unlock(&roulette_mtx);
    }
    return NULL;
}