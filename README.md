Ce projet implémente un serveur de Casino en C utilisant les threads POSIX et les sockets. 
Il permet à plusieurs clients de s'affronter simultanément à la Roulette et au BlackJack.

    Compiler et lancer dans CMD (Première fois) :

    make build

    Ou make up si les images Docker existent déjà.

    Rejoindre le Casino (Joueur 1) dans CMD :

    docker exec -it client ./client_bin

    Rejoindre le Casino (Joueur 2) : Ouvrez un nouveau terminal et tapez la même commande :

    docker exec -it client ./client_bin