/**
 * Implémentation de la Bibliothèque Matrice de Boutons
 */
#include <wiringPi.h>
#include "my_matrice_btn.h"

/**
 * Initialise les broches de la matrice de boutons
 */
void matrice_btn_init(void)
{
    // Configure les broches de ligne en sorties
    pinMode(ROW1, OUTPUT);
    pinMode(ROW2, OUTPUT);
    pinMode(ROW3, OUTPUT);
    pinMode(ROW4, OUTPUT);

    // Configure les broches de colonne en entrées
    pinMode(COL1, INPUT);
    pinMode(COL2, INPUT);
    pinMode(COL3, INPUT);
    pinMode(COL4, INPUT);

    // Initialise les broches de sortie à LOW
    digitalWrite(ROW1, LOW);
    digitalWrite(ROW2, LOW);
    digitalWrite(ROW3, LOW);
    digitalWrite(ROW4, LOW);

    // Active les résistances de tirage sur les entrées
    pullUpDnControl(COL1, PUD_UP);
    pullUpDnControl(COL2, PUD_UP);
    pullUpDnControl(COL3, PUD_UP);
    pullUpDnControl(COL4, PUD_UP);
}

/**
 * Lecture de la matrice de boutons
 * Renvoie le numéro du bouton (1-16) ou 0 si aucun n'est pressé
 */
int matrice_btn_lire(void)
{
    int bouton = 0, ligne;

    // Vérifie chaque ligne
    for (ligne = 0; ligne < 4; ligne++) {
        // Définit toutes les lignes à HIGH (inactive)
        digitalWrite(ROW1, HIGH);
        digitalWrite(ROW2, HIGH);
        digitalWrite(ROW3, HIGH);
        digitalWrite(ROW4, HIGH);

        // Définit la ligne courante à LOW (active)
        switch (ligne) {
            case 0: digitalWrite(ROW1, LOW); break;
            case 1: digitalWrite(ROW2, LOW); break;
            case 2: digitalWrite(ROW3, LOW); break;
            case 3: digitalWrite(ROW4, LOW); break;
        }

        // Court délai pour stabilisation
        delay(2);  // Augmenté pour plus de stabilité

        // Vérifie chaque colonne - Ordre inversé pour correspondre à l'affichage
        if (digitalRead(COL1) == LOW) bouton = ligne * 4 + 4;       // Colonne 4 (droite)
        else if (digitalRead(COL2) == LOW) bouton = ligne * 4 + 3;  // Colonne 3
        else if (digitalRead(COL3) == LOW) bouton = ligne * 4 + 2;  // Colonne 2
        else if (digitalRead(COL4) == LOW) bouton = ligne * 4 + 1;  // Colonne 1 (gauche)

        // Sort si un bouton est trouvé
        if (bouton != 0) break;
    }

    // Réinitialise toutes les lignes à HIGH (état inactif)
    digitalWrite(ROW1, HIGH);
    digitalWrite(ROW2, HIGH);
    digitalWrite(ROW3, HIGH);
    digitalWrite(ROW4, HIGH);

    return bouton;
}

/**
 * Convertit le numéro du bouton en ligne et colonne
 */
void matrice_btn_obtenir_position(int bouton, int *ligne, int *colonne)
{
    if (bouton <= 0 || bouton > 16) {
        *ligne = -1;
        *colonne = -1;
        return;
    }

    bouton--; // Convertit de 1-16 à 0-15
    *ligne = bouton / 4;
    *colonne = bouton % 4;
}

/**
 * Attente d'appui sur un bouton avec temporisation
 */
int matrice_btn_attendre_appui_timeout(int timeout_ms)
{
    int bouton = 0;
    int temps_ecoule = 0;
    int pas_ms = 10;
    
    while (temps_ecoule < timeout_ms || timeout_ms <= 0) {
        bouton = matrice_btn_lire();
        if (bouton != 0) {
            return bouton;
        }
        delay(pas_ms);
        temps_ecoule += pas_ms;
    }
    
    return 0; // Temporisation sans détection
}

/**
 * Attente d'appui sur un bouton sans temporisation
 */
int matrice_btn_attendre_appui(void)
{
    int bouton = 0;

    while (1) {
        bouton = matrice_btn_lire();
        if (bouton != 0) {
            return bouton;
        }
        delay(10);
    }
}

/**
 * Attente de relâchement du bouton avec temporisation
 */
int matrice_btn_attendre_relache_timeout(int timeout_ms)
{
    int bouton = 0;
    int temps_ecoule = 0;
    int pas_ms = 10;

    while (temps_ecoule < timeout_ms || timeout_ms <= 0) {
        bouton = matrice_btn_lire();
        if (bouton == 0) {
            return 1; // Retourne 1 pour un relâchement réussi
        }
        delay(pas_ms);
        temps_ecoule += pas_ms;
    }

    return 0; // Temporisation sans relâchement
}

/**
 * Attente de relâchement du bouton sans temporisation
 */
int matrice_btn_attendre_relache(void)
{
    while (1) {
        if (matrice_btn_lire() == 0) {
            return 1;
        }
        delay(10);
    }
}

/**
 * Nettoie et libère les ressources de la matrice de boutons
 */
void matrice_btn_cleanup(void)
{
    // Réinitialise toutes les lignes à HIGH
    digitalWrite(ROW1, HIGH);
    digitalWrite(ROW2, HIGH);
    digitalWrite(ROW3, HIGH);
    digitalWrite(ROW4, HIGH);
    
    // Configure les broches de sortie en entrée pour éviter les court-circuits
    pinMode(ROW1, INPUT);
    pinMode(ROW2, INPUT);
    pinMode(ROW3, INPUT);
    pinMode(ROW4, INPUT);
    
    // Configure aussi les colonnes en INPUT
    pinMode(COL1, INPUT);
    pinMode(COL2, INPUT);
    pinMode(COL3, INPUT);
    pinMode(COL4, INPUT);

    // Désactive les résistances de tirage
    pullUpDnControl(COL1, PUD_OFF);
    pullUpDnControl(COL2, PUD_OFF);
    pullUpDnControl(COL3, PUD_OFF);
    pullUpDnControl(COL4, PUD_OFF);

    // Désactive aussi les résistances de tirage pour les lignes
    pullUpDnControl(ROW1, PUD_OFF);
    pullUpDnControl(ROW2, PUD_OFF);
    pullUpDnControl(ROW3, PUD_OFF);
    pullUpDnControl(ROW4, PUD_OFF);

    // Petit délai pour laisser les niveaux électriques se stabiliser
    delay(10);
}
