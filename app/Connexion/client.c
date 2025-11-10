#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    int sockfd, n;
    char buffer[256];
    char *host;   // nom ou IP du serveur
    char *mesg;   // message à envoyer

    struct addrinfo hints, *res;
    
    if (argc != 3) {
        fprintf(stderr, "Usage : %s <adresse-serveur> <message>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    host = argv[1];
    mesg = argv[2];

    printf("Adresse du serveur : %s\n", host);
    printf("Message à envoyer : %s\n", mesg);

    // Préparer hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    // Récupérer les infos du serveur
    if (getaddrinfo(host, "5000", &hints, &res) != 0) {
        perror("Erreur getaddrinfo");
        exit(EXIT_FAILURE);
    }

    // Création de la socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("Erreur création socket");
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    // Connexion
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("Erreur connexion au serveur");
        close(sockfd);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    printf("Connexion établie avec le serveur.\n");

    // Envoi du message
    if (write(sockfd, mesg, strlen(mesg)) < 0) {
        perror("Erreur envoi message");
        close(sockfd);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    printf("Message envoyé, attente de la réponse...\n");

    // Lecture de la réponse
    while ((n = read(sockfd, buffer, sizeof(buffer)-1)) > 0) {
        buffer[n] = '\0';
        printf("Réponse du serveur : %s\n", buffer);
    }

    printf("Fin de la communication.\n");

    close(sockfd);
    freeaddrinfo(res);
    return 0;
}
