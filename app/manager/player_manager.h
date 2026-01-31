#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

#include "../entity/player.h"

#define MAX_PLAYERS 128

void init_player_manager();
Player* get_player_by_socket(int sock);
Player* create_player_for_socket(int sock, const char *pseudo);
void remove_player_by_socket(int sock);

#endif