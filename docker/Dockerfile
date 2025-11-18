FROM alpine:latest

# Install compilation tools
RUN apk add --no-cache gcc make musl-dev libc6-compat

WORKDIR /app
COPY . .

# Compile client et serveur
RUN make

# Expose le port serveur
EXPOSE 5000

# Le binaire à exécuter sera choisi par docker-compose via command
CMD ["./serveur_bin"]
