Projet Casino - Sockets C

Application client-serveur permettant de jouer à la roulette et au blackjack en simultané via des threads POSIX.
Prérequis

    GCC

    Make

    Environnement Unix (Linux, macOS)

Instructions de lancement

    Compilation Ouvrez un terminal à la racine du projet et compilez les sources :

    make

    Lancement du serveur Dans le premier terminal, exécutez :

    make run-server

    Lancement des clients Ouvrez un nouveau terminal pour chaque joueur et exécutez :

    make run-client

Nettoyage

Pour supprimer les binaires générés :

make clean