#include <wiringPi.h>
#include <stdio.h>
#include "my_matrice_led.h"

// Définition des formes
int formes[16][8] = {
    {
        // CARRE_GRAND
        0b11111111,
        0b10000001,
        0b10000001,
        0b10000001,
        0b10000001,
        0b10000001,
        0b10000001,
        0b11111111,
    },
    { // CARRE_PETIT
        0b00000000,
        0b00000000,
        0b00111100,
        0b00100100,
        0b00100100,
        0b00111100,
        0b00000000,
        0b00000000,
    },
    { // RECTANGLE_VERTICAL
        0b00111100,
        0b00100100,
        0b00100100,
        0b00100100,
        0b00100100,
        0b00100100,
        0b00100100,
        0b00111100,
    },
    { // RECTANGLE_HORIZONTAL
        0b00000000,
        0b00000000,
        0b11111111,
        0b10000001,
        0b10000001,
        0b11111111,
        0b00000000,
        0b00000000,
    },
    { // TRIANGLE_HAUT
        0b00000000,
        0b00000000,
        0b00011000,
        0b00111100,
        0b01111110,
        0b11111111,
        0b00000000,
        0b00000000,
    },
    { // TRIANGLE_BAS
        0b00000000,
        0b00000000,
        0b11111111,
        0b01111110,
        0b00111100,
        0b00011000,
        0b00000000,
        0b00000000,
    },
    { // TRIANGLE_GAUCHE
        0b00100000,
        0b00110000,
        0b00111000,
        0b00111100,
        0b00111100,
        0b00111000,
        0b00110000,
        0b00100000,
    },
    { // TRIANGLE_DROITE
        0b00000100,
        0b00001100,
        0b00011100,
        0b00111100,
        0b00111100,
        0b00011100,
        0b00001100,
        0b00000100,
    },
    { // CERCLE
        0b00111100,
        0b01000010,
        0b10000001,
        0b10000001,
        0b10000001,
        0b10000001,
        0b01000010,
        0b00111100,
    },
    { // LOSANGE
        0b00011000,
        0b00100100,
        0b01000010,
        0b10000001,
        0b10000001,
        0b01000010,
        0b00100100,
        0b00011000,
    },
    { // BARRE_OBLIQUE_GAUCHE
        0b10000000,
        0b01000000,
        0b00100000,
        0b00010000,
        0b00001000,
        0b00000100,
        0b00000010,
        0b00000001,
    },
    { // BARRE_OBLIQUE_DROITE
        0b00000001,
        0b00000010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b10000000,
    },
    { // CROIX
        0b10000001,
        0b01000010,
        0b00100100,
        0b00011000,
        0b00011000,
        0b00100100,
        0b01000010,
        0b10000001,
    },
    { // LIGNE_HORIZONTALE
        0b00000000,
        0b00000000,
        0b00000000,
        0b11111111,
        0b11111111,
        0b00000000,
        0b00000000,
        0b00000000,
    },
    { // LIGNE_VERTICALE
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
    },
    { // PLUS
        0b00011000,
        0b00011000,
        0b00011000,
        0b11111111,
        0b11111111,
        0b00011000,
        0b00011000,
        0b00011000,
    }
};

// Fonction pour afficher le titre du test
void afficher_titre_test(char* titre) {
    printf("\n=== TEST: %s ===\n", titre);
    delay(1000);
}

// Test de l'initialisation
void test_initialisation() {
    afficher_titre_test("Initialisation");
    matrice_led_init();
    printf("Initialisation terminée\n");
    delay(500);
}

// Test de réglage de luminosité
void test_luminosite() {
    afficher_titre_test("Luminosité");
    
    // Test des différents niveaux de luminosité
    int niveau;
    for (niveau = 0; niveau <= 15; niveau += 3) {
        printf("Luminosité niveau: %d/15\n", niveau);
        matrice_led_luminosite(niveau);
        matrice_led_afficher_motif((int[8]){0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
        delay(1000);
    }
    
    // Retour à la luminosité maximale
    matrice_led_luminosite(15);
    matrice_led_effacer();
}

// Test d'écriture ligne par ligne
void test_ecrire_ligne() {
    afficher_titre_test("Écriture ligne par ligne");

    int ligne;

    // Test d'écriture d'une ligne à la fois
    for (ligne = 0; ligne < 8; ligne++) {
        printf("Écriture ligne: %d\n", ligne);
        matrice_led_ecrire_ligne(ligne, 0xFF);
        delay(500);
    }
    
    delay(1000);
    matrice_led_effacer();
    
    // Test d'écriture de motifs variés par ligne
    for (ligne = 0; ligne < 8; ligne++) {
        printf("Motif alterné ligne: %d\n", ligne);
        matrice_led_ecrire_ligne(ligne, ligne % 2 ? 0xAA : 0x55);
        delay(300);
    }
    
    delay(1000);
    matrice_led_effacer();
}

// Test d'affichage de motifs complets
void test_afficher_motif() {
    afficher_titre_test("Affichage de motifs");

    int i;
    for (i = 0; i < 16; i++) {
        printf("Affichage forme: %d\n", i);
        matrice_led_afficher_motif(formes[i]);
        delay(500);
        matrice_led_effacer();
    }

}

// Test d'animation simple
void test_animation() {
    afficher_titre_test("Animation");

    int i, col, ligne;

    // Animation de balayage horizontal
    printf("Animation: balayage horizontal\n");
    for (i = 0; i < 3; i++) { // Répéter 3 fois
        for (col = 0; col < 8; col++) {
            int motif[8];
            for (ligne = 0; ligne < 8; ligne++) {
                motif[ligne] = 1 << col;
            }
            matrice_led_afficher_motif(motif);
            delay(100);
        }
    }
    
    // Animation de balayage vertical
    printf("Animation: balayage vertical\n");
    for (i = 0; i < 3; i++) { // Répéter 3 fois
        for (ligne = 0; ligne < 8; ligne++) {
            matrice_led_effacer();
            matrice_led_ecrire_ligne(ligne, 0xFF);
            delay(100);
        }
    }
    
    matrice_led_effacer();
}

// Test de la fonction d'effacement
void test_effacement() {
    afficher_titre_test("Effacement");
    
    // Remplir la matrice
    matrice_led_afficher_motif((int[8]){0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
    printf("Matrice remplie\n");
    delay(1000);
    
    // Effacer la matrice
    printf("Effacement...\n");
    matrice_led_effacer();
    printf("Matrice effacée\n");
    delay(1000);
}

int main() {
    wiringPiSetup();
    
    printf("=== DÉBUT DES TESTS DE LA MATRICE LED ===\n");
    
    // Exécuter les tests
    test_initialisation();
    test_luminosite();
    test_ecrire_ligne();
    test_afficher_motif();
    test_animation();
    test_effacement();
    
    printf("=== TESTS TERMINÉS ===\n");
    
    // Nettoyage de la matrice LED
    matrice_led_cleanup();
    
    return 0;
}

