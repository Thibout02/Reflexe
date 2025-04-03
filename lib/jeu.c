#include "jeu.h"
#include "my_7_segment.h"
#include "my_matrice_btn.h"
#include "my_matrice_led.h"
#include "my_ncurses.h"
#include "my_lcd.h"
#include "my_touch.h"

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>

// Définition des formes
extern int formes[16][8];

int envoyer_donnees(int socket, const void *buffer, size_t length) {
    return send(socket, buffer, length, 0);
}

int recevoir_donnees(int socket, void *buffer, size_t length) {
    return recv(socket, buffer, length, 0);
}

int lancer_jeu(void)
{
    srand(time(NULL));

    int temps_final = 0;
    int position_correcte = 0;

    // Initialisation des périphériques
    if (initialiser_peripheriques()) {
        fprintf(stderr, "Erreur d'initialisation des périphériques\n");
        return 1;
    }

    // Affichage message d'accueil sur LCD
    lcd_effacer();
    lcd_ecrire(0, 0, "Jeu de Reflexes");
    lcd_ecrire(0, 1, "Preparez-vous...");
    delay(1000);

    while (1) {
        // Nettoyer et réinitialiser complètement les périphériques entre les parties
        touch_cleanup();
        matrice_btn_cleanup();
        delay(200);

        matrice_btn_init();
        touch_init();
        delay(100);

        // Attendre le contact tactile maintenu pendant une seconde
        attendre_contact_tactile();

        // Désactiver le capteur tactile pendant la partie
        touch_cleanup();
        delay(50);

        // Jouer une partie et récupérer le résultat
        int rep = jouer_partie(&temps_final, &position_correcte);

        // Afficher le résultat
        afficher_resultat(rep, position_correcte, temps_final);

        // Nettoyer les affichages avant de demander si le joueur veut rejouer
        lcd_effacer();
        segment7_effacer();
        matrice_led_effacer();

        int i, j;
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                clear_cell(i, j); // Effacer chaque cellule de la grille
            }
        }
        refresh_display();

        // Réinitialiser la matrice de boutons pour la sélection du menu
        matrice_btn_cleanup();
        delay(100);
        matrice_btn_init();
        delay(50);

        // Demander au joueur s'il veut rejouer ou quitter
        lcd_effacer();
        lcd_ecrire(0, 0, "Rejouer: Btn 1");
        lcd_ecrire(0, 1, "Quitter: Btn 2");

        int choix = 0;
        while (choix != 1 && choix != 2) {
            choix = matrice_btn_lire();
            delay(100); // Pause courte pour éviter de surcharger le CPU
        }

        if (choix == 2) {
            break; // Quitter la boucle et terminer le jeu
        }

    }

    // Nettoyage des périphériques
    nettoyer_peripheriques();

    return 0;
}

int initialiser_peripheriques()
{
    // Initialisation de wiringPi avec vérification plus robuste
    if (wiringPiSetup() == -1) {
        fprintf(stderr, "Erreur critique d'initialisation de wiringPi!\n");
        return 1;
    }

    // Initialisation des autres périphériques avec délais pour stabilisation
    init_ncurses();
    delay(50);

    matrice_btn_init();
    delay(50);

    matrice_led_init();
    delay(50);

    segment7_init();
    delay(50);

    lcd_init();
    delay(50);

    return 0;
}

double attendre_contact_tactile()
{
    // Initialisation du capteur tactile
    reinitialiser_capteur_touch();

    // Message pour demander au joueur de toucher le capteur
    lcd_effacer();
    lcd_ecrire(0, 0, "Maintenez TOUCH");
    lcd_ecrire(0, 1, "pendant 1s");

    // Afficher une barre de progression vide
    int i;
    for (i = 0; i < 16; i++) {
        lcd_position(i, 1);
        lcd_caractere('.');
    }

    // Pause pour stabilisation
    delay(100);

    // Variables pour mesurer précisément le temps de contact
    struct timeval debut_contact, temps_actuel;
    double temps_cumule = 0.0;
    double temps_ecoule;
    int contact_actif = 0;
    int barres_a_afficher = 0;
    int etat_touch = 0;

    // Attendre jusqu'à ce que l'utilisateur maintienne le contact pendant 1 seconde
    while (temps_cumule < 1.0) {
        // Lecture de l'état du capteur
        etat_touch = touch_read();

        // Si le capteur est touché
        if (etat_touch) {
            // Si c'est le début d'un nouveau contact
            if (!contact_actif) {
                contact_actif = 1;
                gettimeofday(&debut_contact, NULL);
            }

            // Obtenir le temps actuel
            gettimeofday(&temps_actuel, NULL);

            // Calculer le temps écoulé depuis le début du contact
            temps_ecoule = (temps_actuel.tv_sec - debut_contact.tv_sec) +
                          ((temps_actuel.tv_usec - debut_contact.tv_usec) / 1000000.0);

            // Mettre à jour le temps cumulé
            temps_cumule = temps_ecoule;
        }
        // Si le capteur n'est pas touché mais était actif avant
        else if (contact_actif) {
            contact_actif = 0;
            temps_cumule = 0;  // Remettre à zéro le compteur
        }

        // Mettre à jour la barre de progression
        barres_a_afficher = (int)(temps_cumule * 16);
        lcd_position(0, 1);
        for (i = 0; i < 16; i++) {
            lcd_caractere(i < barres_a_afficher ? '#' : '.');
        }

        delay(10); // Pause courte pour éviter de surcharger le CPU
    }

    return temps_cumule;
}

int jouer_partie(int *temps_final, int *position_correcte)
{
    // Initialisation de la luminosité de la matrice LED
    matrice_led_luminosite(3);

    // Instructions sur LCD
    lcd_effacer();
    lcd_ecrire(0, 0, "Trouvez la forme");
    lcd_ecrire(0, 1, "sur la grille!");

    // Afficher une forme aléatoire sur la matrice de LED
    int forme = rand() % 16;
    matrice_led_afficher_motif(formes[forme]);

    // Récupère la position de la forme dans la grille (1-16)
    *position_correcte = afficher_grille(forme);

    // Effacer l'afficheur 7 segments et initialiser à 0
    segment7_effacer();
    segment7_afficher_nombre(0);

    // S'assurer que les états des broches sont stables
    matrice_btn_cleanup();
    delay(100);
    matrice_btn_init();
    delay(50);

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
    *temps_final = (int)(temps_secondes * 10);

    return rep; // Retourne la réponse du joueur, pas la position correcte
}

void afficher_resultat(int rep, int position_correcte, int temps_affiche)
{
    char lcd_str[16];

    // Compare le bouton pressé (1-16) avec la position correcte
    if (rep == position_correcte) {
        // Affichage du résultat sur LCD
        lcd_effacer();
        lcd_ecrire(0, 0, "Bravo! Correct!");

        // Formater le temps pour l'affichage LCD
        sprintf(lcd_str, "Temps: %d.%d s", temps_affiche / 10, temps_affiche % 10);
        lcd_ecrire(0, 1, lcd_str);
    } else if (temps_affiche > 99) {
        // Affichage du résultat sur LCD
        lcd_effacer();
        lcd_ecrire(0, 0, "Trop lent!");
        sprintf(lcd_str, "Temps: %d.%d s", temps_affiche / 10, temps_affiche % 10);
        lcd_ecrire(0, 1, lcd_str);
    } else {
        // Affichage du résultat sur LCD
        lcd_effacer();
        lcd_ecrire(0, 0, "Erreur!");
        sprintf(lcd_str, "Position: %d", position_correcte);
        lcd_ecrire(0, 1, lcd_str);
    }

    refresh_display();
    delay(3000);  // Délai plus long pour lire le résultat
    refresh_display();
}

void nettoyer_peripheriques()
{
    // Nettoyage de la matrice LED
    matrice_led_effacer();

    // Nettoyage de ncurses
    cleanup_ncurses();

    // Nettoyage de tous les périphériques
    segment7_effacer();
    segment7_cleanup();

    // Nettoyage de l'écran LCD
    lcd_effacer();
    lcd_cleanup();

    // Nettoyage des GPIO
    touch_cleanup();
    matrice_btn_cleanup();
    matrice_led_cleanup();
}

int afficher_grille(int forme)
{
    // Dessine la grille
    draw_grid();

    char forme_convertie[8][8];
    int i, j, k;
    int position_forme = -1;  // Pour stocker la position de la forme recherchée

    // Tableau pour stocker les indices des formes (0-15)
    int indices[16];

    // Initialisation du tableau d'indices
    for (i = 0; i < 16; i++) {
        indices[i] = i;
    }

    // Mélange aléatoire des indices (Fisher-Yates shuffle)
    int temp;
    for (i = 15; i > 0; i--) {
        j = rand() % (i + 1);
        // Échange des indices i et j
        temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    // Affichage des formes dans la grille selon l'ordre mélangé
    int indice_forme;
    k = 0;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {  // Parcours de gauche à droite (0 à 3)
            indice_forme = indices[k++];
            convertir_forme(formes[indice_forme], forme_convertie);
            display_pattern(i, j, forme_convertie);

            // Si c'est la forme recherchée, enregistrer sa position
            // Calculer la position selon la convention des boutons: ligne*4 + colonne + 1
            if (indice_forme == forme) {
                position_forme = i * 4 + j + 1;  // +1 car les boutons sont numérotés de 1 à 16
            }
        }
    }

    refresh_display();
    return position_forme;  // Retourne la position de la forme (1-16) ou -1 si non trouvée
}

// Nouvelle fonction pour afficher la grille avec un ordre prédéfini des formes
int afficher_grille_predefinie(int forme, const int *indices)
{
    // Dessine la grille
    draw_grid();

    char forme_convertie[8][8];
    int i, j, k;
    int position_forme = -1;  // Pour stocker la position de la forme recherchée

    // Affichage des formes dans la grille selon l'ordre fourni
    int indice_forme;
    k = 0;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {  // Parcours de gauche à droite (0 à 3)
            indice_forme = indices[k++];
            convertir_forme(formes[indice_forme], forme_convertie);
            display_pattern(i, j, forme_convertie);

            // Si c'est la forme recherchée, enregistrer sa position
            // Calculer la position selon la convention des boutons: ligne*4 + colonne + 1
            if (indice_forme == forme) {
                position_forme = i * 4 + j + 1;  // +1 car les boutons sont numérotés de 1 à 16
            }
        }
    }

    refresh_display();
    return position_forme;  // Retourne la position de la forme (1-16) ou -1 si non trouvée
}

void convertir_forme(const int forme[8], char resultat[8][8])
{
    int i, j;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            // Convertit chaque bit en caractère '0' ou '1'
            resultat[i][j] = (forme[i] & (1 << (7 - j))) ? 'x' : ' ';
        }
    }
}

// Fonction à appeler avant chaque nouvelle manche
void reinitialiser_capteur_touch() {
    // Libérer d'abord les ressources
    touch_cleanup();

    // Attendre que les tensions se stabilisent
    delay(200);

    // Réinitialiser et configurer le capteur touch
    touch_init(); // Assurez-vous que cette fonction existe dans votre code

    // Lire et ignorer une première valeur pour "vider" tout état résiduel
    touch_read(); // Assurez-vous que cette fonction existe dans votre code

    // Autre délai pour s'assurer que tout est stable
    delay(100);
}

