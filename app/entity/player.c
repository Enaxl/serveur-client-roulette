#include "player.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

Player create_player(const char *pseudo, int client_socket) {
    Player p;
    strncpy(p.pseudo, pseudo, sizeof(p.pseudo) - 1);
    p.pseudo[sizeof(p.pseudo) - 1] = '\0';
    p.client_socket = client_socket;
    p.total_coins = 1000;
    p.connection_time = time(NULL);

    return p;
}

void add_coins(Player *p, int amount) {
    p->total_coins += amount;
}

bool check_coins(Player *p, int amount) {
    return p->total_coins >= amount;
}

int remove_coins(Player *p, int amount) {
    if (p->total_coins < amount)
        return 0;
    p->total_coins -= amount;
    return 1;
}

void print_player(const Player *p) {
    printf("Joueur : %s\n", p->pseudo);
    printf("Coins : %d\n", p->total_coins);
    printf("Connecté depuis : %ld secondes\n", p->connection_time);
}
