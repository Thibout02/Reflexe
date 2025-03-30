/**
 * Implémentation de la Bibliothèque Matrice de LEDs
 */
#include <wiringPi.h>

#include "my_matrice_led.h"

/**
 * Fonction interne pour écrire un octet de données au MAX7219
 */
void bit_write(int donnee) {
    int i;
    for (i = 0; i < 8; i++) {
        digitalWrite(SCLK, LOW);
        digitalWrite(MOSI, (donnee & 0x80) ? HIGH : LOW);
        donnee <<= 1;
        digitalWrite(SCLK, HIGH);
    }
}

/**
 * Initialiser la matrice de LED
 */
void matrice_led_init(void) {
    pinMode(MOSI, OUTPUT);
    pinMode(SCLK, OUTPUT);
    pinMode(GPIO7, OUTPUT);

    // Configuration initiale
    matrice_led_envoyer_cmd(REG_SHUTDOWN, 0x01);     // Fonctionnement normal
    matrice_led_envoyer_cmd(REG_DECODE_MODE, 0x00);  // Mode sans décodage
    matrice_led_envoyer_cmd(REG_SCAN_LIMIT, 0x07);   // Afficher tous les digits (0-7)
    matrice_led_envoyer_cmd(REG_INTENSITY, 15);      // Luminosité maximale
    matrice_led_envoyer_cmd(REG_DISPLAY_TEST, 0x00); // Fonctionnement normal (pas de test)

    // Effacer l'affichage
    matrice_led_effacer();
}

/**
 * Envoyer une commande au MAX7219
 */
void matrice_led_envoyer_cmd(int adresse, int donnee) {
    delay(25); // Court délai pour éviter les problèmes de communication
    digitalWrite(GPIO7, LOW);
    bit_write(adresse);
    bit_write(donnee);
    digitalWrite(GPIO7, HIGH);
}

/**
 * Afficher un motif sur une ligne spécifique
 */
void matrice_led_ecrire_ligne(int index_ligne, int motif) {
    if (index_ligne < 8) {
        matrice_led_envoyer_cmd(REG_DIGIT0 + index_ligne, motif);
    }
}

/**
 * Afficher un motif sur toute la matrice
 */
void matrice_led_afficher_motif(int motif[8]) {
    int i;
    for (i = 0; i < 8; i++) {
        matrice_led_ecrire_ligne(i, motif[i]);
    }
}

/**
 * Effacer l'affichage
 */
void matrice_led_effacer(void) {
    int motif_zero[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    matrice_led_afficher_motif(motif_zero);
}

/**
 * Régler l'intensité de la luminosité
 */
void matrice_led_luminosite(int intensite) {
    if (intensite > 15) intensite = 15;
    matrice_led_envoyer_cmd(REG_INTENSITY, intensite);
}

/**
 * Nettoie et libère les ressources de la matrice de LED
 */
void matrice_led_cleanup(void) {
    // Efface l'affichage
    matrice_led_effacer();
    
    // Met l'afficheur en mode extinction
    matrice_led_envoyer_cmd(REG_SHUTDOWN, 0x00);
    
    // Réinitialise les broches en mode entrée
    pinMode(MOSI, INPUT);
    pinMode(SCLK, INPUT);
    pinMode(GPIO7, INPUT);
}
