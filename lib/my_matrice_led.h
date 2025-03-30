/**
 * Bibliothèque Matrice de LEDs
 */
#ifndef MATRICE_LED_H
#define MATRICE_LED_H

// Registres MAX7219
#define REG_NOOP         0x00 // Pas d'opération
#define REG_DIGIT0       0x01 // Digit 0
#define REG_DECODE_MODE  0x09 // Mode de décodage
#define REG_INTENSITY    0x0A // Intensité
#define REG_SCAN_LIMIT   0x0B // Limite de balayage
#define REG_SHUTDOWN     0x0C // Arrêt
#define REG_DISPLAY_TEST 0x0F // Test d'affichage

// Broches matrice de LED
#define MOSI  12
#define SCLK  14
#define GPIO7 11

// Initialiser la matrice de LED
void matrice_led_init(void);

// Envoyer une commande au MAX7219
void matrice_led_envoyer_cmd(int adresse, int donnee);

// Afficher un motif sur une ligne spécifique
void matrice_led_ecrire_ligne(int index_ligne, int motif);

// Afficher un motif sur toute la matrice
void matrice_led_afficher_motif(int motif[8]);

// Effacer l'affichage
void matrice_led_effacer(void);

// Régler l'intensité de la luminosité (0-15)
void matrice_led_luminosite(int intensite);

// Nettoie et libère les ressources de la matrice de LED
void matrice_led_cleanup(void);

#endif /* MATRICE_LED_H */
