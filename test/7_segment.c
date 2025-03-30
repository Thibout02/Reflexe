#include <stdio.h>
#include <wiringPi.h>
#include "my_7_segment.h"

int main() {
    int i;
    printf("Test de l'afficheur à 7 segments simplifié\n");
    
    // Initialiser wiringPi
    if (wiringPiSetup() == -1) {
        printf("Initialisation de WiringPi échouée\n");
        return 1;
    }

    // Initialiser l'afficheur
    segment7_init();
    printf("Afficheur initialisé\n");

    // Tester l'affichage des nombres
    printf("Test d'affichage des nombres...\n");
    for (i = 0; i < 10000; i += 1111) {
        printf("Affichage: %d\n", i);
        segment7_afficher_nombre(i);
        delay(500);
    }
    
    // Tester un nombre négatif
    printf("Affichage d'un nombre négatif: -42\n");
    segment7_afficher_nombre(-42);
    delay(1000);
    
    // Tester les points décimaux
    printf("Test des points décimaux...\n");
    segment7_effacer();
    delay(1000);
    
    // Afficher un nombre avec points décimaux
    segment7_afficher_nombre(1234);
    
    printf("Ajout d'un point décimal (1.234)\n");
    segment7_decimal(0, 1);  // Position 0 = premier chiffre
    delay(500);

    printf("Ajout d'un point décimal (12.34)\n");
    segment7_decimal(1, 1);  // Position 1 = deuxième chiffre
    delay(500);

    printf("Ajout d'un point décimal (123.4)\n");
    segment7_decimal(2, 1);  // Position 2 = troisième chiffre
    delay(500);

    printf("Ajout d'un point décimal (1234.)\n");
    segment7_decimal(3, 1);  // Position 3 = quatrième chiffre
    delay(500);
    
    // Tester les deux-points (style horloge)
    printf("Test des deux-points...\n");
    segment7_afficher_nombre(1234);
    
    for (i = 0; i < 5; i++) {
        segment7_deuxpoints(1);
        delay(500);
        segment7_deuxpoints(0);
        delay(500);
    }
    
    // Tester les niveaux de luminosité
    printf("Test des niveaux de luminosité...\n");
    segment7_afficher_nombre(8888);
    
    for (i = 0; i <= 15; i++) {
        printf("Luminosité: %d/15\n", i);
        segment7_luminosite(i);
        delay(200);
    }
    
    // Effacer pour terminer
    segment7_effacer();
    printf("Test terminé\n");
    
    return 0;
}
