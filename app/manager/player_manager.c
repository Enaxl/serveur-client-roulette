#include "player_manager.h"
#include <string.h>
#include <pthread.h>

// Fichier qui nous permet de gérer la gestion des joueurs au fil du temps

static Player players[MAX_PLAYERS];
static int player_count = 0;
pthread_mutex_t players_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_player_manager() {
    pthread_mutex_lock(&players_mutex);
    player_count = 0;
    pthread_mutex_unlock(&players_mutex);
}

// Création d'un joueur
Player* create_player_for_socket(int sock, const char *pseudo) {
    pthread_mutex_lock(&players_mutex);
    if (player_count >= MAX_PLAYERS) {
        pthread_mutex_unlock(&players_mutex);
        return NULL;
    }
    players[player_count] = create_player(pseudo, sock);
    Player* p = &players[player_count++];
    pthread_mutex_unlock(&players_mutex);
    return p;
}


// Supprimer un joueur
void remove_player_by_socket(int sock) {
    pthread_mutex_lock(&players_mutex);
    for (int i = 0; i < player_count; i++) {
        if (players[i].client_socket == sock) {
            players[i] = players[player_count - 1];
            player_count--;
            break;
        }
    }
    pthread_mutex_unlock(&players_mutex);
}