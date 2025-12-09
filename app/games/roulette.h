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
void handle_roulette(Player *p, const char *command);
const char *get_parity(int number);

#endif
