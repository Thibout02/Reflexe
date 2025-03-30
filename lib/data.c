/**
 *  \file       data.c
 *  \brief      Implémentation des fonctions de gestion des données pour les sockets
 *  \author     Thibaut CAZIER
 *  \date       31 janvier 2025
 */

#include "data.h"

#include <stdarg.h>
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
 *  \fn         void envoyer(socket_t *sockEch, generic quoi, pFct serial, ...)
 *  \brief      Envoie d'une requête/réponse sur une socket
 *  \param      sockEch Socket d'échange à utiliser pour l'envoi
 *  \param      quoi Requête/réponse à sérialiser avant envoi
 *  \param      serial Pointeur de fonction de sérialisation de la requête/réponse
 *  \note       Si le paramètre serial vaut NULL, alors quoi est une chaîne de caractères
 *  \note       Si le mode est DGRAM, l'appel nécessite en plus une adresse IP et un port
 */
void envoyer(socket_t *sockEch, generic quoi, pFct serial, ...) {
    buffer_t buffer;
    struct sockaddr_in addrDst;
    ssize_t len;

    // Si serial n'est pas NULL, on sérialise les données
    if (serial != NULL) {
        serial(quoi, buffer);
    } else {
        strncpy(buffer, (char *)quoi, sizeof(buffer));
    }

    // Si mode est DGRAM, on utilise des paramètres supplémentaires (IP et port)
    if (sockEch->mode == SOCK_DGRAM) {
        va_list args;
        va_start(args, serial);
        addrDst = va_arg(args, struct sockaddr_in);
        va_end(args);

        len = sendto(sockEch->fd, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&addrDst, sizeof(addrDst));
        if (len == -1) {
            perror("Erreur lors de l'envoi (DGRAM)");
            return;
        }
    } else if (sockEch->mode == SOCK_STREAM) { // STREAM
        len = send(sockEch->fd, buffer, strlen(buffer) + 1, 0);
        if (len == -1) {
            perror("Erreur lors de l'envoi (STREAM)");
            return;
        }
    } else {
        fprintf(stderr, "Mode socket non supporté\n");
    }
}

/**
 *  \fn         void recevoir(socket_t *sockEch, generic quoi, pFct deSerial)
 *  \brief      Réception d'une requête/réponse sur une socket
 *  \param      sockEch Socket d'échange
 *  \param      quoi Donnée à recevoir
 *  \param      deSerial Fonction de désérialisation
 */
void recevoir(socket_t *sockEch, generic quoi, pFct deSerial) {
    buffer_t buffer;
    ssize_t recu;
    struct sockaddr_in addrExp;
    socklen_t addrLen = sizeof(addrExp);

    // Réception selon le mode
    if (sockEch->mode == SOCK_DGRAM) { // DGRAM
        recu = recvfrom(sockEch->fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addrExp, &addrLen);
        if (recu == -1) {
            perror("Erreur lors de la réception (DGRAM)");
            return;
        }
    } else if (sockEch->mode == SOCK_STREAM) { // STREAM
        recu = recv(sockEch->fd, buffer, sizeof(buffer), 0);
        if (recu == -1) {
            perror("Erreur lors de la réception (STREAM)");
            return;
        }
    } else {
        fprintf(stderr, "Mode socket non supporté\n");
        return;
    }

    buffer[recu] = '\0'; // Terminer correctement la chaîne reçue

    // Si deSerial n'est pas NULL, on désérialise les données
    if (deSerial != NULL) {
        deSerial(buffer, quoi);
    } else {
        strncpy((char *)quoi, buffer, sizeof(buffer));
    }
}

/**
 *  \fn         void str2reponse(buffer_t message, reponse_t *reponse)
 *  \brief      Convertir une chaîne de caractères en requête
 *  \param      message Chaîne de caractères à convertir
 *  \param      reponse Requête à remplir
 */
void str2reponse(buffer_t message, reponse_t *reponse) {
    char *token = strtok(message, ":");
    if (token != NULL) {
        reponse->id = atoi(token);
        token = strtok(NULL, "");
        if (token != NULL) {
            strncpy(reponse->message, token, sizeof(reponse->message));
        }
    }
}

/**
 *  \fn         void reponse2str(reponse_t *reponse, buffer_t message)
 *  \brief      Convertir une requête en chaîne de caractères
 *  \param      reponse Requête à convertir
 *  \param      message Chaîne de caractères à remplir
 */
void reponse2str(reponse_t *reponse, buffer_t message) {
    sprintf(message, "%d:%s", reponse->id, reponse->message);
}