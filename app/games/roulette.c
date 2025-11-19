#include "roulette.h"
#include "../serveur/network.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>


void handle_roulette(Player *p, const char *command) {
    char reply[256];
    int bet_amount = 0;
    char bet_type[32];
    char bet_value[64];

    // 1) Parsing : "MISE <montant> number 17"
    if (sscanf(command, "MISE %d %31s %63s", &bet_amount, bet_type, bet_value) != 3) {
        snprintf(reply, sizeof(reply),
                 "Format invalide.\nExemple: MISE 10 number 17\n");
        send_message(p->client_socket, reply);
        return;
    }

    // déduction immédiate
    remove_coins(p, bet_amount);

    // 3) On lance la roulette
    RouletteResult result = spin_roulette();
    int gain = 0;

    // 4) Résolution du pari
    if (strcmp(bet_type, "number") == 0) {
        int chosen = atoi(bet_value);
        if (chosen == result.number) gain = bet_amount * 36;
    }
    else if (strcmp(bet_type, "color") == 0) {
        if (strcmp(bet_value, result.color) == 0) gain = bet_amount * 2;
    }
    else if (strcmp(bet_type, "parity") == 0) {
        if (strcmp(bet_value, result.parity) == 0) gain = bet_amount * 2;
    }
    else {
        snprintf(reply, sizeof(reply), "Type de pari inconnu.\n");
        send_message(p->client_socket, reply);
        return;
    }

    // 5) On ajoute le gain (si il y en a un)
    if (gain > 0) {
        add_coins(p, gain);
    }

    // 6) Message final
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
    if (number == 0) return "vert";
    return (number % 2 == 0) ? "noir" : "rouge";
}

const char *get_parity(int number) {
    if (number == 0) return "zéro";
    return (number % 2 == 0) ? "pair" : "impair";
}

RouletteResult spin_roulette() {
    static int init = 0;
    if (!init) { srand(time(NULL)); init = 1; }

    int n = rand() % 37;
    RouletteResult res;
    res.number = n;
    res.color = get_color(n);
    res.parity = get_parity(n);
    return res;
}
