#ifndef BLACKJACK_H
#define BLACKJACK_H

#include "../entity/player.h"

typedef struct {
    int score;
    int cards_count;
} BlackJackHand;

void handle_blackjack_waiting(Player *p);
void* blackjack_timer_thread(void* arg);
void process_bj_command(Player *p, const char* cmd);
void leave_blackjack(Player *p);

#endif