# Utiliser l'image Alpine Linux comme base
FROM alpine:latest

# Installation des outils de développement C et des bibliothèques nécessaires
RUN apk update && \
    apk add --no-cache gcc musl-dev make

# Création d'un répertoire de travail
WORKDIR /socket

# Copie du code source dans le conteneur
COPY /css/ /socket/css/
COPY /html/ /socket/html/
COPY /include/ /socket/include/
COPY /src/ /socket/src/
COPY Makefile /socket/Makefile

EXPOSE 80

# Compilation du code source
RUN make

# Commande par défaut pour démarrer le serveur
CMD ["./http.o"]
