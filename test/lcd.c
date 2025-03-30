#include <stdio.h>
#include <unistd.h>
#include "my_lcd.h"

int main() {
    int lcd;

    // Initialisation du LCD
    printf("Initialisation du LCD...\n");
    lcd = lcd_init();
    if (lcd < 0) {
        printf("Erreur d'initialisation du LCD\n");
        return 1;
    }

    printf("LCD initialisé avec succès! Handle: %d\n", lcd);

    // Test de base
    lcd_effacer();
    lcd_ecrire(0, 0, "Test LCD");
    lcd_ecrire(0, 1, "Ligne 2");
    sleep(2);

    // Test avec printf
    lcd_effacer();
    lcd_position(0, 0);
    lcd_printf("Valeur: %d", 42);
    lcd_position(0, 1);
    lcd_printf("PI: %.2f", 3.14159);
    sleep(2);

    // Test du curseur
    lcd_effacer();
    lcd_ecrire(0, 0, "Test curseur");
    lcd_curseur(1);
    lcd_position(0, 1);
    sleep(1);

    // Test du clignotement du curseur
    lcd_curseur_clignote(1);
    sleep(2);
    lcd_curseur_clignote(0);
    lcd_curseur(0);

    // Test de caractère personnalisé (un cœur)
    unsigned char heart[8] = {
        0b00000,
        0b01010,
        0b11111,
        0b11111,
        0b11111,
        0b01110,
        0b00100,
        0b00000
    };

    lcd_def_caractere(0, heart);
    lcd_effacer();
    lcd_ecrire(0, 0, "Caractere special:");
    lcd_position(0, 1);
    lcd_caractere(0); // Utilisation du caractère personnalisé
    sleep(2);

    // Animation simple
    lcd_effacer();
    lcd_ecrire(0, 0, "Animation:");

    int i;
    for (i = 0; i < 16; i++) {
        lcd_position(i, 1);
        lcd_caractere(0);
        usleep(200000); // 200ms
    }

    // Message final
    lcd_effacer();
    lcd_ecrire(0, 0, "Test termine!");
    lcd_ecrire(0, 1, "avec succes");
    sleep(2);
    
    lcd_effacer();
    
    // Nettoyage du LCD
    lcd_cleanup();

    return 0;
}
