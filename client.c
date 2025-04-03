#include "jeu.h"
#include "my_lcd.h"
#include "my_touch.h"
#include "my_matrice_btn.h"
#include "my_matrice_led.h"
#include "my_ncurses.h"
#include "my_7_segment.h"

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/time.h>    // Ajout de cet en-tête pour la fonction gettimeofday

#define SERVER_IP "192.168.1.19"
#define PORT 12345

extern int formes[16][8];

int main() {
    // Initialisation des périphériques
    if (initialiser_peripheriques()) {
        fprintf(stderr, "Erreur d'initialisation des périphériques\n");
        return EXIT_FAILURE;
    }
    
    // Création du socket client
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Erreur lors de la création du socket");
        nettoyer_peripheriques();
        return EXIT_FAILURE;
    }
    
    // Configuration de l'adresse du serveur
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);
    
    // Connexion au serveur
    lcd_effacer();
    lcd_ecrire(0, 0, "Connexion au");
    lcd_ecrire(0, 1, "serveur...");
    
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur de connexion au serveur");
        lcd_effacer();
        lcd_ecrire(0, 0, "Erreur connexion");
        lcd_ecrire(0, 1, "serveur!");
        delay(2000);
        close(client_socket);
        nettoyer_peripheriques();
        return EXIT_FAILURE;
    }
    
    lcd_effacer();
    lcd_ecrire(0, 0, "Connecte!");
    lcd_ecrire(0, 1, "En attente...");
    delay(1000);
    
    bool continuer = true;
    while (continuer) {
        // Nettoyer et réinitialiser complètement les périphériques entre les parties
        touch_cleanup();
        matrice_btn_cleanup();
        delay(200);

        matrice_btn_init();
        touch_init();
        delay(100);
        
        // Attendre que le joueur maintienne le bouton tactile pendant 1 seconde
        attendre_contact_tactile();
        
        // Envoyer la confirmation au serveur que le bouton a été maintenu
        char confirmation = '1';
        envoyer_donnees(client_socket, &confirmation, 1);
        
        lcd_effacer();
        lcd_ecrire(0, 0, "Attente des");
        lcd_ecrire(0, 1, "autres joueurs..");
        
        // Attendre que tous les autres joueurs aient aussi maintenu le bouton
        char synchro;
        recevoir_donnees(client_socket, &synchro, 1);
        
        if (synchro != '1') {
            lcd_effacer();
            lcd_ecrire(0, 0, "Erreur de synchro");
            lcd_ecrire(0, 1, "avec le serveur");
            delay(2000);
            break;
        }
        
        // Recevoir la forme à chercher
        int forme;
        recevoir_donnees(client_socket, &forme, sizeof(int));
        
        // Recevoir également l'ordre des indices
        int indices[16];
        recevoir_donnees(client_socket, indices, sizeof(indices));

        // Variables pour stocker les résultats
        int temps_final = 0;
        int position_correcte = 0;
        
        // Afficher la forme à chercher sur la matrice LED
        matrice_led_afficher_motif(formes[forme]);

        // Utiliser la nouvelle fonction pour afficher la grille avec l'ordre prédéfini
        position_correcte = afficher_grille_predefinie(forme, indices);

        // Informer le joueur
        lcd_effacer();
        lcd_ecrire(0, 0, "Cherchez la forme");
        lcd_ecrire(0, 1, "dans la grille!");
        
        // Effacer l'afficheur 7 segments et initialiser à 0
        segment7_effacer();
        segment7_afficher_nombre(0);

        // Jouer la partie (on utilise une version modifiée de jouer_partie)
        // Cette partie est commune avec le jeu local, mais nous n'utilisons pas la fonction
        // jouer_partie directement car elle inclut déjà l'appel à afficher_grille

        // Démarrage du chronomètre
        struct timeval debut, maintenant;
        gettimeofday(&debut, NULL);

        int rep = 0;
        int temps_affiche = 0;
        int temps_precedent = -1;
        double temps_ecoule;

        // Boucle du chronomètre qui s'incrémente jusqu'à ce qu'un bouton soit pressé
        while (rep == 0 && temps_affiche < 100) {
            // Vérifier si un bouton a été pressé
            rep = matrice_btn_lire();

            // Mettre à jour le chronomètre toutes les 100ms
            gettimeofday(&maintenant, NULL);
            temps_ecoule = (maintenant.tv_sec - debut.tv_sec) +
                           ((maintenant.tv_usec - debut.tv_usec) / 1000000.0);

            // Convertir en dixième de seconde
            temps_affiche = (int)(temps_ecoule * 10);

            // Mettre à jour l'affichage seulement si le temps a changé
            if (temps_affiche != temps_precedent) {
                segment7_afficher_nombre(temps_affiche);
                temps_precedent = temps_affiche;
            }

            // Petite pause pour ne pas surcharger le CPU
            delay(10);
        }

        // Calculer le temps final
        gettimeofday(&maintenant, NULL);
        double temps_secondes = (maintenant.tv_sec - debut.tv_sec) +
                               ((maintenant.tv_usec - debut.tv_usec) / 1000000.0);
        temps_final = (int)(temps_secondes * 10);
        
        // Vérifier si la réponse est correcte
        bool reponse_correcte = (rep == position_correcte);
        
        // Afficher le résultat local
        char message[16];
        if (!reponse_correcte) {
            lcd_effacer();
            lcd_ecrire(0, 0, "Mauvaise reponse!");
            sprintf(message, "Position: %d", position_correcte);
            lcd_ecrire(0, 1, message);
        } else if (temps_final > 99) {
            lcd_effacer();
            lcd_ecrire(0, 0, "Trop lent!");
            sprintf(message, "Temps: %d.%ds", temps_final/10, temps_final%10);
            lcd_ecrire(0, 1, message);
        } else {
            lcd_effacer();
            sprintf(message, "Temps: %d.%ds", temps_final/10, temps_final%10);
            lcd_ecrire(0, 0, message);
            lcd_ecrire(0, 1, "Attente resultat");
        }
        
        // Envoyer d'abord si la réponse est correcte
        int est_correct = reponse_correcte ? 1 : 0;
        envoyer_donnees(client_socket, &est_correct, sizeof(int));
        
        // Puis envoyer le temps au serveur
        envoyer_donnees(client_socket, &temps_final, sizeof(int));
        
        // Recevoir les résultats comparatifs
        int meilleur_temps, pire_temps;
        recevoir_donnees(client_socket, &meilleur_temps, sizeof(int));
        recevoir_donnees(client_socket, &pire_temps, sizeof(int));

        // Nettoyer l'affichage
        matrice_led_effacer();
        cleanup_ncurses();
        
        // Afficher les résultats comparatifs
        delay(2000); // Attendre 2 secondes pour que le joueur puisse voir son résultat
        lcd_effacer();
        sprintf(message, "Meilleur: %d.%ds", meilleur_temps/10, meilleur_temps%10);
        lcd_ecrire(0, 0, message);
        sprintf(message, "Pire: %d.%ds", pire_temps/10, pire_temps%10);
        lcd_ecrire(0, 1, message);
        delay(3000);
        
        // Demander si le joueur veut rejouer
        lcd_effacer();
        lcd_ecrire(0, 0, "Rejouer ?");
        lcd_ecrire(0, 1, "O:BTN1  N:BTN2");
        
        // Attendre que le joueur appuie sur un bouton
        int choix = 0;
        while (choix != 1 && choix != 2) {
            choix = matrice_btn_lire();
            delay(100);
        }
        
        // Traiter le choix
        char reponse = (choix == 1) ? 'o' : 'n';
        envoyer_donnees(client_socket, &reponse, 1);
        
        // Si le joueur ne veut pas rejouer, sortir de la boucle
        if (reponse == 'n') {
            continuer = false;
            lcd_effacer();
            lcd_ecrire(0, 0, "Fin du jeu!");
            lcd_ecrire(0, 1, "Au revoir!");
        }
    }
    
    // Fermer le socket et nettoyer les périphériques
    close(client_socket);
    nettoyer_peripheriques();
    
    return EXIT_SUCCESS;
}
