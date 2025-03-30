/**
 *  \file       session.c
 *  \brief      Implémentation des fonctions de gestion des sessions pour les sockets
 *  \author     Thibaut CAZIER
 *  \date       31 janvier 2025
 */

#include "session.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

#define CHECK(sts, msg) if ((sts) == -1) { perror(msg); exit(1); }

#ifdef DEBUG
    #define PAUSE(msg) {printf("%s [Appuyez sur entrée pour continuer]\n", msg); getchar();}
#else
    #define PAUSE(msg) {}
#endif

/**
 *  \fn         void adr2struct(struct sockaddr_in *addr, char *adrIP, int port)
 *  \brief      Transformer une adresse au format humain en structure SocketBSD
 *  \param      addr Structure d'adressage BSD d'une socket INET
 *  \param      adrIP Adresse IP de la socket à créer
 *  \param      port Port de la socket à créer
 */
void adr2struct(struct sockaddr_in *addr, char *adrIP, int port) {
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = inet_addr(adrIP);
    memset(&(addr->sin_zero), 0, 8);
}

/**
 *  \fn         socket_t creerSocket(int mode)
 *  \brief      Création d'une socket de type DGRAM ou STREAM
 *  \param      mode Mode de la socket (SOCK_DGRAM ou SOCK_STREAM)
 *  \return     La socket créée
 */
socket_t creerSocket(int mode) {
    socket_t sock;
    CHECK(sock.fd = socket(AF_INET, mode, 0), "socket");
    sock.mode = mode;
    return sock;
}

/**
 *  \fn         socket_t creerSocketAdr(int mode, char *adrIP, int port)
 *  \brief      Création d'une socket de type DGRAM ou STREAM avec une adresse
 *  \param      mode Mode de la socket (SOCK_DGRAM ou SOCK_STREAM)
 *  \param      adrIP Adresse IP
 *  \param      port Port
 *  \return     La socket créée
 */
socket_t creerSocketAdr(int mode, char *adrIP, int port) {
    socket_t sock;
    sock = creerSocket(mode);
    adr2struct(&sock.addrLoc, adrIP, port);
    CHECK(bind(sock.fd, (struct sockaddr *) &sock.addrLoc, sizeof(sock.addrLoc)), "Can't bind");
    return sock;
}

/**
 *  \fn         socket_t creerSocketEcoute(char *adrIP, int port, int nbMax)
 *  \brief      Création d'une socket d'écoute
 *  \param      adrIP Adresse IP
 *  \param      port Port
 *  \param      nbMax Nombre maximum de clients en attente
 *  \return     La socket créée
 */
socket_t creerSocketEcoute(char *adrIP, int port, int nbMax) {
    socket_t sock;
    sock = creerSocketAdr(SOCK_STREAM, adrIP, port);
    CHECK(listen(sock.fd, nbMax), "listen"); // Augmenter la taille de la file d'attente
    return sock;
}

/**
 *  \fn         socket_t accepterClt(const socket_t sockEcoute)
 *  \brief      Accepter un client sur une socket d'écoute
 *  \param      sockEcoute Socket d'écoute
 *  \return     La socket de dialogue
 */
socket_t accepterClt(socket_t sockEcoute) {
    socket_t sockDial;
    socklen_t lg = sizeof(sockDial.addrDst);
    CHECK(sockDial.fd = accept(sockEcoute.fd, (struct sockaddr *)&sockDial.addrDst, &lg), "accept");
    sockDial.mode = sockEcoute.mode;
    sockDial.addrLoc = sockEcoute.addrLoc;
    return sockDial;
}

/**
 *  \fn         socket_t connecterClt2Srv(char *adrIP, int port)
 *  \brief      Connecter un client à un serveur
 *  \param      adrIP Adresse IP du serveur
 *  \param      port Port du serveur
 *  \return     La socket de dialogue
 */
socket_t connecterClt2Srv(char *adrIP, int port) {
    socket_t sock;
    sock = creerSocket(SOCK_STREAM);
    adr2struct(&(sock.addrLoc), adrIP, port);
    CHECK(connect(sock.fd, (struct sockaddr *)&sock.addrLoc, sizeof(sock.addrLoc)), "connect");
    return sock;
}

/**
 *  \fn         void fermeSocket(socket_t sock)
 *  \brief      Fermer une socket
 *  \param      sock Socket à fermer
 */
void fermeSocket(socket_t sock) {
    CHECK(close(sock.fd), "close");
}