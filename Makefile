# -------------------
# Compilateur et flags
# -------------------
CC = gcc

# L'ajout de -D_POSIX_C_SOURCE=200112L règle ton erreur de compilation sur "addrinfo"
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200112L -pthread
LDFLAGS = -pthread

# Répertoires d'inclusion
INCLUDES = -Iapp/client -Iapp/serveur -Iapp/entity -Iapp/manager -Iapp/games

# Noms des exécutables
CLIENT_BIN = client_bin
SERVEUR_BIN = serveur_bin

# -------------------
# Sources
# -------------------
CLIENT_SRC = app/client/client.c app/serveur/network.c

SERVEUR_SRC = app/serveur/serveur.c app/serveur/network.c \
              app/entity/player.c app/manager/player_manager.c \
              app/games/roulette.c app/games/blackjack.c

# -------------------
# Règles de compilation
# -------------------

all: $(CLIENT_BIN) $(SERVEUR_BIN)

# Compilation du client
$(CLIENT_BIN):
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(CLIENT_SRC) $(LDFLAGS)

# Compilation du serveur
$(SERVEUR_BIN):
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(SERVEUR_SRC) $(LDFLAGS)

clean:
	rm -f $(CLIENT_BIN) $(SERVEUR_BIN)

# -------------------
# Aide à l'exécution (Local)
# -------------------

run-server: $(SERVEUR_BIN)
	./$(SERVEUR_BIN)

run-client: $(CLIENT_BIN)
	./$(CLIENT_BIN)