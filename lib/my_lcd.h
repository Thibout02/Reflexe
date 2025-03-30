#ifndef LCD_H
#define LCD_H

// Adresse du périphérique et constantes
#define ADRESSE_LCD 0x21
#define MAX_LCDS 8
#define MAX_CAR 1000

// Définitions des broches GPIO pour MCP23008
#define AF_BASE 100
#define AF_RS (AF_BASE+1)
#define AF_STRB (AF_BASE+2)
#define AF_DB4 (AF_BASE+3)
#define AF_DB5 (AF_BASE+4)
#define AF_DB6 (AF_BASE+5)
#define AF_DB7 (AF_BASE+6)
#define AF_BLUE (AF_BASE+12)
#define AF_RW (AF_BASE+14)
#define AF_BACKLIGHT_PIN (AF_BASE+15)
#define AF_BACKLIGHTGRD_PIN (AF_BASE+16)

// Broches I2C
#define SDA 3
#define SCL 5

// Initialisation de l'écran LCD
int lcd_init(void);

// Écrire un message à une position spécifique
void lcd_ecrire(int colonne, int ligne, char* message);

// Effacer l'écran
void lcd_effacer(void);

// Positionner le curseur
void lcd_position(int x, int y);

// Retourner à la position d'origine
void lcd_origine(void);

// Contrôle de l'affichage
void lcd_affichage(int etat);

// Contrôle du curseur
void lcd_curseur(int etat);

// Contrôle du clignotement du curseur
void lcd_curseur_clignote(int etat);

// Écrire un caractère
void lcd_caractere(unsigned char c);

// Écrire une chaîne de caractères
void lcd_chaine(char *chaine);

// Écrire avec formatage
void lcd_printf(char *format, ...);

// Définir un caractère personnalisé
void lcd_def_caractere(int index, unsigned char data[8]);

// Envoyer une commande directe
void lcd_commande(unsigned char commande);

// Libère les ressources de l'écran LCD
void lcd_cleanup(void);

#endif /* LCD_H */
