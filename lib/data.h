/**
 *  \file       data.h
 *  \brief      Déclarations des fonctions et types pour la gestion des données des sockets
 *  \author     Thibaut CAZIER
 *  \date       31 janvier 2025
 */

#include "session.h"

/**
 *  \def        MAX_BUFFER
 *  \brief      Taille maximale du buffer
 */
#define MAX_BUFFER 1024

/**
 *  \typedef    buffer_t
 *  \brief      Chaîne de caractères à émettre/recevoir
 */
typedef char buffer_t[MAX_BUFFER];

/**
 *  \typedef    generic
 *  \brief      Type de données générique : requête/réponse
 */
typedef void* generic;

/**
 *  \typedef    pFct
 *  \brief      Pointeur de fonction générique à deux paramètres
 */
typedef void (*pFct) (generic, generic);

/**
 *  \struct     reponse_t
 *  \brief      Structure de données pour les requêtes/réponses
 *  \var        reponse_t.id
 *              Identifiant de la requête/réponse
 *  \var        reponse_t.message
 *              Message de la requête/réponse
 */
typedef struct {
    int   id;
    buffer_t  message;
} reponse_t;

/**
 *  \fn         void envoyer(socket_t *sockEch, generic quoi, pFct serial, ...)
 *  \brief      Envoyer une requête/réponse sur une socket
 *  \param      sockEch Socket d'échange à utiliser pour l'envoi
 *  \param      quoi Requête/réponse à sérialiser avant envoi
 *  \param      serial Pointeur de fonction de sérialisation de la requête/réponse
 *  \note       Si le paramètre serial vaut NULL, alors quoi est une chaîne de caractères
 *  \note       Si le mode est DGRAM, l'appel nécessite en plus une adresse IP et un port
 */
void envoyer(socket_t *sockEch, generic quoi, pFct serial, ...);

/**
 *  \fn         void recevoir(socket_t *sockEch, generic quoi, pFct deSerial)
 *  \brief      Réception d'une requête/réponse sur une socket
 *  \param      sockEch Socket d'échange
 *  \param      quoi Donnée à recevoir
 *  \param      deSerial Fonction de désérialisation
 */
void recevoir(socket_t *sockEch, generic quoi, pFct deSerial);

/**
 *  \fn         void str2reponse(buffer_t message, reponse_t *reponse)
 *  \brief      Convertir une chaîne de caractères en requête
 *  \param      message Chaîne de caractères à convertir
 *  \param      reponse Requête à remplir
 */
void str2reponse(buffer_t message, reponse_t *reponse);

/**
 *  \fn         void reponse2str(reponse_t *reponse, buffer_t message)
 *  \brief      Convertir une requête en chaîne de caractères
 *  \param      reponse Requête à convertir
 *  \param      message Chaîne de caractères à remplir
 */
void reponse2str(reponse_t *reponse, buffer_t message);