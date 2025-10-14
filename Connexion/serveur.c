#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define TAILLE_MAX_NOM 256

void renvoi(int sock) {
    char buffer[256];
    int longueur;

    if ((longueur = read(sock, buffer, sizeof(buffer) - 1)) <= 0)
        return;

    buffer[longueur] = '\0';
    printf("message lu : %s\n", buffer);

    buffer[0] = 'R';
    buffer[1] = 'E';
    if (longueur >= sizeof(buffer) - 2) longueur = sizeof(buffer) - 3;
    buffer[longueur] = '#';
    buffer[longueur + 1] = '\0';

    printf("message apres traitement : %s\n", buffer);
    printf("renvoi du message traite.\n");

    sleep(3);
    write(sock, buffer, strlen(buffer) + 1);
    printf("message envoye.\n");
}

int main(int argc, char **argv) {
    int socket_descriptor, nouv_socket_descriptor;
    socklen_t longueur_adresse_courante;
    struct sockaddr_in adresse_locale, adresse_client_courant;
    char machine[TAILLE_MAX_NOM + 1];

    gethostname(machine, TAILLE_MAX_NOM);
    printf("Nom de la machine : %s\n", machine);

    adresse_locale.sin_family = AF_INET;
    adresse_locale.sin_addr.s_addr = INADDR_ANY;
    adresse_locale.sin_port = htons(5000);

    printf("numero de port pour la connexion au serveur : %d\n", ntohs(adresse_locale.sin_port));

    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("erreur : impossible de creer la socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(socket_descriptor, (struct sockaddr *)&adresse_locale, sizeof(adresse_locale)) < 0) {
        perror("erreur : impossible de lier la socket");
        exit(1);
    }

    listen(socket_descriptor, 5);

    for (;;) {
        longueur_adresse_courante = sizeof(adresse_client_courant);
        if ((nouv_socket_descriptor = accept(socket_descriptor,
                                              (struct sockaddr *)&adresse_client_courant,
                                              &longueur_adresse_courante)) < 0) {
            perror("erreur : impossible d'accepter la connexion");
            exit(1);
        }
        printf("reception d'un message.\n");
        renvoi(nouv_socket_descriptor);
        close(nouv_socket_descriptor);
    }

    close(socket_descriptor);
    return 0;
}
