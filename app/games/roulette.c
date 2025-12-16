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
                 "Format invalide.\nExemple: MISE 10 number 17\n"");
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
        snprintf(reply, sizeof(reply), "Type de pari inconnu.\n"");
        send_line(p->client_socket, reply);
        return;
    }

    // Ajout du gain
    if (gain > 0) {
        add_coins(p, gain);
    }

    // Message final (TOUT EN UNE SEULE LIGNE)
    snprintf(reply, sizeof(reply),
        "Résultat : %d %s %s\n"
        "Mise : %d | Gain : %d\n"
        "Coins restants : %d\n"",
        result.number, result.color, result.parity,
        bet_amount, gain, p->total_coins
    );

    send_line(p->client_socket, reply);  // envoie tout en une seule fois
}
