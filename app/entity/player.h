#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

typedef struct {
    int client_socket;
    char pseudo[32];
    int total_coins;
    long connection_time;
} Player;

Player create_player(const char *pseudo, int client_socket);

void add_coins(Player *p, int amount);
bool check_coins(Player *p, int amount);
int remove_coins(Player *p, int amount);

void print_player(const Player *p);

#endif
