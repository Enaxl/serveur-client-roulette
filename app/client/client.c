#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include "../serveur/network.h"

// Variable globale pour stocker la saisie en cours
char current_input[256] = {0};
pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;

void* receive_thread(void* arg) {
    int sock = *((int*)arg);
    char buffer[1024];
    while (1) {
        int n = receive_line(sock, buffer, sizeof(buffer));
        if (n <= 0) {
            printf("\n[DECONNEXION] Le serveur a ferme la connexion.\n");
            exit(0);
        }

        if (strcmp(buffer, "END_MSG\n") != 0) {
            pthread_mutex_lock(&display_mutex);
            // \r = retour début de ligne, \033[K = efface la ligne
            printf("\r\033[K%s", buffer);
            // On réaffiche l'invite et ce que l'utilisateur était en train de taper
            printf("Commande > %s", current_input);
            fflush(stdout);
            pthread_mutex_unlock(&display_mutex);
        }
    }
    return NULL;
}

int main() {
    char buffer[512];
    setvbuf(stdout, NULL, _IONBF, 0);

    int sock = connect_to_server("serveur", 5000);
    printf("Connecte au Casino !\n");

    // Phase Pseudo (Bloquante au début)
    receive_line(sock, buffer, sizeof(buffer));
    printf("%s ", buffer);

    char pseudo[64];
    fgets(pseudo, sizeof(pseudo), stdin);
    pseudo[strcspn(pseudo, "\n")] = 0;
    send_line(sock, pseudo);

    // Lancement du thread de réception
    pthread_t tid;
    pthread_create(&tid, NULL, receive_thread, &sock);

    // Boucle principale de saisie
    while (1) {
        pthread_mutex_lock(&display_mutex);
        printf("\r\033[KCommande > ");
        fflush(stdout);
        pthread_mutex_unlock(&display_mutex);

        // On utilise fgets pour la saisie
        if (fgets(current_input, sizeof(current_input), stdin)) {
            current_input[strcspn(current_input, "\n")] = 0;

            if (strlen(current_input) > 0) {
                send_line(sock, current_input);

                // On vide le buffer global après l'envoi
                pthread_mutex_lock(&display_mutex);
                memset(current_input, 0, sizeof(current_input));
                pthread_mutex_unlock(&display_mutex);
            }
        } else {
            break;
        }
    }

    close_socket(sock);
    return 0;
}