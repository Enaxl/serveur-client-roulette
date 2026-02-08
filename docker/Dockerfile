FROM alpine:latest

RUN apk add --no-cache gcc make musl-dev libc6-compat

WORKDIR /app

COPY . .

RUN make clean && make

EXPOSE 5000

CMD ["./serveur_bin"]