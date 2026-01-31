#ifndef ROULETTE_H
#define ROULETTE_H

#include "../entity/player.h"

typedef struct {
    int number;
    const char *color;
    const char *parity;
} RouletteResult;

RouletteResult spin_roulette();
const char *get_color(int number);
const char *get_parity(int number);

void handle_roulette_waiting(Player *p);
void leave_roulette(Player *p);
void place_roulette_bet(Player *p, int amount, const char* type, const char* value);
void* roulette_timer_thread(void* arg);

#endif