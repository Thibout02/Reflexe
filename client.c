#include "jeu.h"
#include "my_lcd.h"
#include "my_matrice_btn.h"
#include "my_matrice_led.h"
#include "my_ncurses.h"

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define SERVER_IP "192.168.1.19"
#define PORT 12345

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
        // Affichage du message d'attente
        lcd_effacer();
        lcd_ecrire(0, 0, "Maintenez TOUCH");
        lcd_ecrire(0, 1, "pendant 1s");
        
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
        
        // Variables pour stocker les résultats
        int temps_final = 0;
        int position_correcte = 0;
        
        // Jouer la partie
        lcd_effacer();
        lcd_ecrire(0, 0, "Cherchez la forme");
        lcd_ecrire(0, 1, "dans la grille!");
        
        int rep = jouer_partie(&temps_final, &position_correcte);
        
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
            lcd_ecrire(0, 1, "Attente resultats");
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
