/**
 * Implémentation de la Bibliothèque pour Afficheur à 7 Segments
 */
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <unistd.h>  // Ajout de cet en-tête pour la fonction close()
#include "my_7_segment.h"

// Motifs de segments pour les chiffres 0-9
// Format: DP,G,F,E,D,C,B,A (DP=point décimal)
int DIGIT_SEGMENTS[] = {
    0x3F, // 0 = 0b00111111
    0x06, // 1 = 0b00000110
    0x5B, // 2 = 0b01011011
    0x4F, // 3 = 0b01001111
    0x66, // 4 = 0b01100110
    0x6D, // 5 = 0b01101101
    0x7D, // 6 = 0b01111101
    0x07, // 7 = 0b00000111
    0x7F, // 8 = 0b01111111
    0x6F  // 9 = 0b01101111
};

// Constantes de commande
#define CMD_SYSTEM 0x20
#define CMD_DISPLAY 0x80
#define CMD_BRIGHTNESS 0xE0
#define DECIMAL_BIT 0x80

// Mappage des positions pour l'affichage
const int POSITION_MAP[4] = {0, 1, 3, 4};

// Buffer d'affichage et descripteur de fichier I2C
int display_buffer[8] = {0};
int i2c_fd = -1;

/**
 * Fonctions internes
 */
// Envoyer une commande à l'afficheur
void envoyer_commande(int cmd) {
    if (i2c_fd >= 0) {
        wiringPiI2CWrite(i2c_fd, cmd);
    }
}

/**
 * Mettre à jour l'affichage avec le contenu actuel du buffer
 */
void maj_affichage(void) {
    if (i2c_fd < 0) return;

    int i;
    for (i = 0; i < 8; i++) {
        wiringPiI2CWriteReg8(i2c_fd, i*2, display_buffer[i] & 0xFF);
        wiringPiI2CWriteReg8(i2c_fd, i*2+1, display_buffer[i] >> 8);
    }
}

/**
 * Initialiser l'afficheur
 */
void segment7_init(void) {
    i2c_fd = wiringPiI2CSetup(HT16K33_ADDRESS);
    
    if (i2c_fd < 0) {
        printf("Erreur : Initialisation I2C échouée\n");
        return;
    }
    
    // Activer l'oscillateur
    envoyer_commande(CMD_SYSTEM | 0x01);
    
    // Allumer l'afficheur
    envoyer_commande(CMD_DISPLAY | 0x01);
    
    // Régler à la luminosité maximale
    segment7_luminosite(15);
    
    // Effacer l'affichage
    segment7_effacer();
}

/**
 * Régler le niveau de luminosité (0-15)
 */
void segment7_luminosite(int luminosite) {
    if (luminosite > 15) luminosite = 15;
    if (luminosite < 0) luminosite = 0;
    envoyer_commande(CMD_BRIGHTNESS | luminosite);
}

/**
 * Effacer l'afficheur
 */
void segment7_effacer(void) {
    int i;
    for (i = 0; i < 8; i++) {
        display_buffer[i] = 0;
    }
    maj_affichage();
}

/**
 * Écrire un chiffre à la position spécifiée
 */
void segment7_ecrire_chiffre(int position, int valeur) {
    // Vérifier les limites
    if (position < 0 || position >= MAX_DIGITS || valeur < 0 || valeur > 9) {
        return;
    }
    
    // Préserver l'état du point décimal
    int buffer_pos = POSITION_MAP[position];
    int decimal = display_buffer[buffer_pos] & DECIMAL_BIT;
    display_buffer[buffer_pos] = DIGIT_SEGMENTS[valeur] | decimal;
    maj_affichage();
}

/**
 * Activer/désactiver le point décimal à la position
 */
void segment7_decimal(int position, int actif) {
    if (position < 0 || position >= MAX_DIGITS) {
        return;
    }
    
    int buffer_pos = POSITION_MAP[position];
    if (actif) {
        display_buffer[buffer_pos] |= DECIMAL_BIT;
    } else {
        display_buffer[buffer_pos] &= ~DECIMAL_BIT;
    }
    maj_affichage();
}

/**
 * Activer/désactiver les deux-points
 */
void segment7_deuxpoints(int actif) {
    if (actif) {
        display_buffer[2] |= 0x02;
    } else {
        display_buffer[2] &= ~0x02;
    }
    maj_affichage();
}

/**
 * Afficher un nombre
 */
void segment7_afficher_nombre(int nombre) {
    // Effacer l'affichage
    segment7_effacer();
    
    // Cas spécial pour zéro
    if (nombre == 0) {
        segment7_ecrire_chiffre(MAX_DIGITS - 1, 0);
        return;
    }
    
    // Gérer les nombres négatifs
    int est_negatif = (nombre < 0);
    if (est_negatif) nombre = -nombre;
    
    // Afficher les chiffres de droite à gauche
    int position = MAX_DIGITS - 1;
    while (nombre > 0 && position >= 0) {
        segment7_ecrire_chiffre(position, nombre % 10);
        nombre /= 10;
        position--;
    }
    
    // Afficher le signe moins si nécessaire
    if (est_negatif && position >= 0) {
        display_buffer[POSITION_MAP[position]] = 0x40; // Segment G uniquement
        maj_affichage();
    }
}

/**
 * Nettoie et libère les ressources de l'afficheur 7 segments
 */
void segment7_cleanup(void) {
    // Efface l'affichage
    segment7_effacer();
    
    // Désactive l'oscillateur (met l'afficheur en mode veille)
    envoyer_commande(CMD_SYSTEM | 0x00);
    
    // Éteint l'afficheur
    envoyer_commande(CMD_DISPLAY | 0x00);
    
    // Ferme le descripteur de fichier I2C si nécessaire
    if (i2c_fd >= 0) {
        close(i2c_fd);
        i2c_fd = -1;
    }
}

