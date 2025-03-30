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



int lancer_jeu(void)
{
    // Initialisation de wiringPi
    if (wiringPiSetup() == -1) {
        printf("Erreur d'initialisation de wiringPi\n");
        return 1;
    }

    srand(time(NULL));

    int temps_final = 0;
    int position_correcte = 0;
    
    // Initialisation des périphériques
    initialiser_peripheriques();

    // Affichage message d'accueil sur LCD
    lcd_effacer();
    lcd_ecrire(0, 0, "Jeu de Reflexes");
    lcd_ecrire(0, 1, "Preparez-vous...");
    delay(1000);
    
    // Attendre le contact tactile maintenu pendant une seconde
    attendre_contact_tactile();
    
    // Jouer une partie et récupérer le résultat
    int rep = jouer_partie(&temps_final, &position_correcte);
    
    // Afficher le résultat
    afficher_resultat(rep, position_correcte, temps_final);
    
    // Nettoyage des périphériques
    nettoyer_peripheriques();

    return 0;
}

void initialiser_peripheriques()
{
    // Initialisation de ncurses et des périphériques
    init_ncurses();
    matrice_btn_init();
    matrice_led_init();
    segment7_init();
    touch_init();
    lcd_init();
}

double attendre_contact_tactile()
{
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

    // Petite pause pour stabiliser l'affichage
    delay(500);

    // Variables pour mesurer précisément le temps de contact
    struct timeval debut_contact, temps_actuel;
    double temps_cumule = 0.0;
    double temps_ecoule;
    int contact_actif = 0;
    int barres_a_afficher = 0;
    int etat_touch = 0;

    while (temps_cumule < 1.0) { // 1.0 seconde
        // Lecture de l'état du capteur
        etat_touch = touch_read();

        // Obtenir le temps actuel
        gettimeofday(&temps_actuel, NULL);

        // Si le capteur est touché
        if (etat_touch) {
            // Si c'est le début d'un nouveau contact
            if (!contact_actif) {
                contact_actif = 1;
                gettimeofday(&debut_contact, NULL);
                // Ajuster debut_contact pour tenir compte du temps déjà cumulé
                debut_contact.tv_sec -= (int)temps_cumule;
                debut_contact.tv_usec -= (temps_cumule - (int)temps_cumule) * 1000000;
                if (debut_contact.tv_usec < 0) {
                    debut_contact.tv_sec--;
                    debut_contact.tv_usec += 1000000;
                }
            }

            // Calculer le temps écoulé depuis le début du contact
            temps_ecoule = (temps_actuel.tv_sec - debut_contact.tv_sec) +
                           ((temps_actuel.tv_usec - debut_contact.tv_usec) / 1000000.0);

            // Mettre à jour le temps cumulé
            temps_cumule = temps_ecoule;
        }
        // Si le capteur n'est pas touché mais était actif avant
        else if (contact_actif) {
            contact_actif = 0;
            // Remise à zéro du temps cumulé
            temps_cumule = 0;
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
