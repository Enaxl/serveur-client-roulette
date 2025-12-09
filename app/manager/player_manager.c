#include "player_manager.h"
#include <string.h>

static Player players[MAX_PLAYERS];
static int player_count = 0;

void init_player_manager() {
    player_count = 0;
}

Player* get_player_by_socket(int sock) {
    for (int i = 0; i < player_count; i++)
        if (players[i].client_socket == sock)
            return &players[i];

    return NULL;
}

Player* create_player_for_socket(int sock, const char *pseudo) {
    if (player_count >= MAX_PLAYERS) return NULL;

    players[player_count] = create_player(pseudo, sock);
    return &players[player_count++];
}
