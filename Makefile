COMPOSE := docker compose -f docker/docker-compose.yml
EXEC    := $(COMPOSE) exec

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

cc: ## Clear cache
	$(CONSOLE) cache:clear

