/**
 *  \file       session.h
 *  \brief      Déclarations des fonctions et types pour la gestion des sessions de sockets
 *  \author     Thibaut CAZIER
 *  \date       31 janvier 2025
 */

#include <netinet/in.h>

/**
 *  \typedef    socket_t
 *  \brief      Structure représentant une socket
 *  \var        socket_t.fd
 *              Descripteur de fichier de la socket
 *  \var        socket_t.mode
 *              Mode de la socket (SOCK_DGRAM ou SOCK_STREAM)
 *  \var        socket_t.addrLoc
 *              Adresse locale de la socket
 *  \var        socket_t.addrDst
 *              Adresse distante de la socket
 */
typedef struct {
    int fd;
    int mode;
    struct sockaddr_in addrLoc;
    struct sockaddr_in addrDst;
} socket_t;

/**
 *  \fn         void adr2struct(struct sockaddr_in *addr, char *adrIP, int port)
 *  \brief      Transformer une adresse au format humain en structure SocketBSD
 *  \param      addr Structure d'adressage BSD d'une socket INET
 *  \param      adrIP Adresse IP de la socket à créer
 *  \param      port Port de la socket à créer
 */
void adr2struct(struct sockaddr_in *addr, char *adrIP, int port);

/**
 *  \fn         socket_t creerSocket(int mode)
 *  \brief      Création d'une socket de type DGRAM ou STREAM
 *  \param      mode Mode de la socket (SOCK_DGRAM ou SOCK_STREAM)
 *  \return     La socket créée
 */
socket_t creerSocket(int mode);

/**
 *  \fn         socket_t creerSocketAdr(int mode, char *adrIP, int port)
 *  \brief      Création d'une socket de type DGRAM ou STREAM avec une adresse
 *  \param      mode Mode de la socket (SOCK_DGRAM ou SOCK_STREAM)
 *  \param      adrIP Adresse IP
 *  \param      port Port
 *  \return     La socket créée
 */
socket_t creerSocketAdr(int mode, char *adrIP, int port);

/**
 *  \fn         socket_t creerSocketEcoute(char *adrIP, int port, int nbMax)
 *  \brief      Création d'une socket d'écoute
 *  \param      adrIP Adresse IP
 *  \param      port Port
 *  \param      nbMax Nombre maximum de clients en attente
 *  \return     La socket créée
 */
socket_t creerSocketEcoute(char *adrIP, int port, int nbMax);

/**
 *  \fn         socket_t accepterClt(const socket_t sockEcoute)
 *  \brief      Accepter un client sur une socket d'écoute
 *  \param      sockEcoute Socket d'écoute
 *  \return     La socket de dialogue
 */
socket_t accepterClt(socket_t sockEcoute);

/**
 *  \fn         socket_t connecterClt2Srv(char *adrIP, int port)
 *  \brief      Connecter un client à un serveur
 *  \param      adrIP Adresse IP du serveur
 *  \param      port Port du serveur
 *  \return     La socket de dialogue
 */
socket_t connecterClt2Srv(char *adrIP, int port);

/**
 *  \fn         void fermeSocket(socket_t sock)
 *  \brief      Fermer une socket
 *  \param      sock Socket à fermer
 */
void fermeSocket(socket_t sock);