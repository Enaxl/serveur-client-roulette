#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    int socket_descriptor, longueur;
    struct sockaddr_in adresse_locale;
    struct hostent *ptr_host;
    char buffer[256];
    char *host;  /* nom ou IP du serveur */
    char *mesg;  /* message envoyé */

    if (argc != 3) {
        fprintf(stderr, "Usage : %s <adresse-serveur> <message>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    host = argv[1];
    mesg = argv[2];

    printf("Adresse du serveur : %s\n", host);
    printf("Message à envoyer : %s\n", mesg);

    /* récupération de l'adresse IP à partir du nom */
    if ((ptr_host = gethostbyname(host)) == NULL) {
        perror("Erreur : impossible de trouver le serveur à partir de son adresse");
        exit(EXIT_FAILURE);
    }

    /* création de la structure d'adresse */
    memset(&adresse_locale, 0, sizeof(adresse_locale));
    adresse_locale.sin_family = AF_INET;
    memcpy(&adresse_locale.sin_addr, ptr_host->h_addr, ptr_host->h_length);
    adresse_locale.sin_port = htons(5000);  // même port que le serveur

    printf("Numéro de port utilisé : %d\n", ntohs(adresse_locale.sin_port));

    /* création de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erreur : impossible de créer la socket");
        exit(EXIT_FAILURE);
    }

    /* tentative de connexion */
    if (connect(socket_descriptor, (struct sockaddr *)&adresse_locale, sizeof(adresse_locale)) < 0) {
        perror("Erreur : impossible de se connecter au serveur");
        close(socket_descriptor);
        exit(EXIT_FAILURE);
    }

    printf("Connexion établie avec le serveur.\n");

    /* envoi du message */
    if (write(socket_descriptor, mesg, strlen(mesg)) < 0) {
        perror("Erreur : impossible d'envoyer le message au serveur");
        close(socket_descriptor);
        exit(EXIT_FAILURE);
    }

    printf("Message envoyé au serveur, attente de la réponse...\n");

    /* lecture de la réponse */
    while ((longueur = read(socket_descriptor, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[longueur] = '\0';
        printf("Réponse du serveur : %s\n", buffer);
    }

    printf("Fin de la communication.\n");
    close(socket_descriptor);
    return 0;
}
