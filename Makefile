COMPOSE := docker compose -f docker/docker-compose.yaml
EXEC    := $(COMPOSE) exec
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

CLIENT_SRC = app/client/client.c
SERVER_SRC = app/serveur/serveur.c

all: client_bin serveur_bin

client_bin: $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $^

serveur_bin: $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f client_bin serveur_bin

up:
	$(COMPOSE) up -d

down:
	$(COMPOSE) down

logs:
	$(COMPOSE) logs -f app

build:
	$(COMPOSE) up -d --build --remove-orphans

ps:
	$(COMPOSE) ps


