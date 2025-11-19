# -------------------
# Compilateur et flags
# -------------------
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

INCLUDES = -Iapp/client -Iapp/serveur -Iapp/entity -Iapp/manager -Iapp/games

CLIENT_BIN = client_bin
SERVEUR_BIN = serveur_bin

# -------------------
# Sources
# -------------------
CLIENT_SRC = app/client/client.c app/serveur/network.c
SERVEUR_SRC = app/serveur/serveur.c app/serveur/network.c \
              app/entity/player.c app/manager/player_manager.c app/games/roulette.c

# -------------------
# Règles de compilation
# -------------------
all: $(CLIENT_BIN) $(SERVEUR_BIN)

$(CLIENT_BIN):
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(CLIENT_SRC)

$(SERVEUR_BIN):
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(SERVEUR_SRC)

clean:
	rm -f $(CLIENT_BIN) $(SERVEUR_BIN)

# -------------------
# Docker / Compose
# -------------------
COMPOSE = docker compose -f docker/docker-compose.yaml
EXEC    = $(COMPOSE) exec

up:
	$(COMPOSE) up -d

down:
	$(COMPOSE) down

build:
	$(COMPOSE) up -d --build --remove-orphans

logs:
	$(COMPOSE) logs -f

ps:
	$(COMPOSE) ps
