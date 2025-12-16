#include "roulette.h"
#include "../serveur/network.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

void handle_roulette(Player *p, const char *command) {
    char reply[512];  // plus grand pour tout le message
    int bet_amount = 0;
    char bet_type[32];
    char bet_value[64];

    // Parsing : "MISE <montant> number 17"
    if (sscanf(command, "MISE %d %31s %63s", &bet_amount, bet_type, bet_value) != 3) {
        snprintf(reply, sizeof(reply),
                 "Format invalide.\nExemple: MISE 10 number 17\n");
        send_line(p->client_socket, reply);
        return;
    }

    remove_coins(p, bet_amount);

    RouletteResult result = spin_roulette();
    int gain = 0;

    // Résolution du pari
    if (strcmp(bet_type, "number") == 0) {
        int chosen = atoi(bet_value);
        if (chosen == result.number) gain = bet_amount * 35;
    } else if (strcmp(bet_type, "color") == 0) {
        if (strcmp(bet_value, result.color) == 0) gain = bet_amount * 2;
    } else if (strcmp(bet_type, "parity") == 0) {
        if (strcmp(bet_value, result.parity) == 0) gain = bet_amount * 2;
    } else {
        snprintf(reply, sizeof(reply), "Type de pari inconnu.\n");
        send_line(p->client_socket, reply);
        return;
    }

    // Ajout du gain
    if (gain > 0) {
        add_coins(p, gain);
    }

    snprintf(reply, sizeof(reply),
        "Résultat : %d %s %s\n"
        "Mise : %d | Gain : %d\n"
        "Coins restants : %d\n",
        result.number, result.color, result.parity,
        bet_amount, gain, p->total_coins
    );

    send_message(p->client_socket, reply);
}

const char *get_color(int number) {
    if (number == 0) {
        return "vert";
    }

    return (number % 2 == 0) ? "noir" : "rouge";
}

const char *get_parity(int number) {
    if (number == 0) {
        return "zéro";
    }

    return (number % 2 == 0) ? "pair" : "impair";
}

RouletteResult spin_roulette(void) {
    static int initialized = 0;

    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }

    RouletteResult result;
    result.number = rand() % 37;
    result.color = get_color(result.number);
    result.parity = get_parity(result.number);

    return result;
}
