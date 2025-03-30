/**
 * Implémentation de la Bibliothèque pour écran LCD I2C
 */
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>
#include <mcp23008.h>
#include <wiringPiI2C.h>
#include "my_lcd.h"

#ifndef TRUE
#define TRUE  (1==1)
#define FALSE (1==2)
#endif

// Commandes LCD HD44780U
#define LCD_CLEAR   0x01
#define LCD_HOME    0x02
#define LCD_ENTRY   0x04
#define LCD_CTRL    0x08
#define LCD_CDSHIFT 0x10
#define LCD_FUNC    0x20
#define LCD_CGRAM   0x40
#define LCD_DGRAM   0x80

// Bits du registre d'entrée
#define LCD_ENTRY_SH    0x01
#define LCD_ENTRY_ID    0x02

// Bits du registre de contrôle
#define LCD_BLINK_CTRL    0x01
#define LCD_CURSOR_CTRL   0x02
#define LCD_DISPLAY_CTRL  0x04

// Bits du registre de fonction
#define LCD_FUNC_F  0x04
#define LCD_FUNC_N  0x08
#define LCD_FUNC_DL 0x10

#define LCD_CDSHIFT_RL 0x04

// Structure de données LCD
struct lcdDataStruct {
  int bits, rows, cols;
  int rsPin, strbPin;
  int dataPins[8];
  int cx, cy;
};

// Variables globales
struct lcdDataStruct *lcds[MAX_LCDS];
int lcdControl;
static int lcdHandle = -1;

// Décalages pour les lignes
int rowOff[4] = { 0x00, 0x40, 0x14, 0x54 };

/**
 * Fonctions internes
 */

/**
 * Envoyer une impulsion sur la broche strobe (E)
 */
void strobe(struct lcdDataStruct *lcd) {
  digitalWrite(lcd->strbPin, 1); delayMicroseconds(50);
  digitalWrite(lcd->strbPin, 0); delayMicroseconds(50);
}

/**
 * Envoyer une donnée ou commande à l'écran
 */
void sendDataCmd(struct lcdDataStruct *lcd, unsigned char data) {
  register unsigned char myData = data;
  unsigned char i, d4;

  if (lcd->bits == 4) {
    d4 = (myData >> 4) & 0x0F;
    for (i = 0; i < 4; ++i) {
      digitalWrite(lcd->dataPins[i], (d4 & 1));
      d4 >>= 1;
    }
    strobe(lcd);

    d4 = myData & 0x0F;
    for (i = 0; i < 4; ++i) {
      digitalWrite(lcd->dataPins[i], (d4 & 1));
      d4 >>= 1;
    }
  } else {
    for (i = 0; i < 8; ++i) {
      digitalWrite(lcd->dataPins[i], (myData & 1));
      myData >>= 1;
    }
  }
  strobe(lcd);
}

/**
 * Envoyer une commande à l'écran
 */
void putCommand(struct lcdDataStruct *lcd, unsigned char command) {
  digitalWrite(lcd->rsPin, 0);
  sendDataCmd(lcd, command);
  delay(2);
}

/**
 * Envoyer une commande 4 bits à l'écran
 */
void put4Command(struct lcdDataStruct *lcd, unsigned char command) {
  register unsigned char myCommand = command;
  register unsigned char i;

  digitalWrite(lcd->rsPin, 0);

  for (i = 0; i < 4; ++i) {
    digitalWrite(lcd->dataPins[i], (myCommand & 1));
    myCommand >>= 1;
  }
  strobe(lcd);
}

/**
 * Fonctions publiques
 */

/**
 * Positionner le curseur à l'origine
 */
void lcd_origine() {
  struct lcdDataStruct *lcd = lcds[lcdHandle];

  putCommand(lcd, LCD_HOME);
  lcd->cx = lcd->cy = 0;
  delay(5);
}

/**
 * Effacer l'écran
 */
void lcd_effacer() {
  struct lcdDataStruct *lcd = lcds[lcdHandle];

  putCommand(lcd, LCD_CLEAR);
  putCommand(lcd, LCD_HOME);
  lcd->cx = lcd->cy = 0;
  delay(5);
}

/**
 * Activer/désactiver l'affichage
 */
void lcd_affichage(int etat) {
  struct lcdDataStruct *lcd = lcds[lcdHandle];

  if (etat)
    lcdControl |= LCD_DISPLAY_CTRL;
  else
    lcdControl &= ~LCD_DISPLAY_CTRL;

  putCommand(lcd, LCD_CTRL | lcdControl);
}

/**
 * Activer/désactiver le curseur
 */
void lcd_curseur(int etat) {
  struct lcdDataStruct *lcd = lcds[lcdHandle];

  if (etat)
    lcdControl |= LCD_CURSOR_CTRL;
  else
    lcdControl &= ~LCD_CURSOR_CTRL;

  putCommand(lcd, LCD_CTRL | lcdControl);
}

/**
 * Activer/désactiver le clignotement du curseur
 */
void lcd_curseur_clignote(int etat) {
  struct lcdDataStruct *lcd = lcds[lcdHandle];

  if (etat)
    lcdControl |= LCD_BLINK_CTRL;
  else
    lcdControl &= ~LCD_BLINK_CTRL;

  putCommand(lcd, LCD_CTRL | lcdControl);
}

/**
 * Envoyer une commande arbitraire à l'écran
 */
void lcd_commande(unsigned char command) {
  struct lcdDataStruct *lcd = lcds[lcdHandle];
  putCommand(lcd, command);
}

/**
 * Positionner le curseur
 */
void lcd_position(int x, int y) {
  struct lcdDataStruct *lcd = lcds[lcdHandle];

  if ((x > lcd->cols) || (x < 0))
    return;
  if ((y > lcd->rows) || (y < 0))
    return;

  putCommand(lcd, x + (LCD_DGRAM | rowOff[y]));

  lcd->cx = x;
  lcd->cy = y;
}

/**
 * Définir un caractère personnalisé
 */
void lcd_def_caractere(int index, unsigned char data[8]) {
  struct lcdDataStruct *lcd = lcds[lcdHandle];
  int i;

  putCommand(lcd, LCD_CGRAM | ((index & 7) << 3));

  digitalWrite(lcd->rsPin, 1);
  for (i = 0; i < 8; ++i)
    sendDataCmd(lcd, data[i]);
}

/**
 * Écrire un caractère
 */
void lcd_caractere(unsigned char data) {
  struct lcdDataStruct *lcd = lcds[lcdHandle];

  digitalWrite(lcd->rsPin, 1);
  sendDataCmd(lcd, data);

  if (++lcd->cx == lcd->cols) {
    lcd->cx = 0;
    if (++lcd->cy == lcd->rows)
      lcd->cy = 0;
    
    putCommand(lcd, lcd->cx + (LCD_DGRAM | rowOff[lcd->cy]));
  }
}

/**
 * Écrire une chaîne de caractères
 */
void lcd_chaine(char *chaine) {
  while (*chaine)
    lcd_caractere(*chaine++);
}

/**
 * Écrire avec formatage
 */
void lcd_printf(char *format, ...) {
  va_list argp;
  char buffer[1024];

  va_start(argp, format);
  vsnprintf(buffer, 1023, format, argp);
  va_end(argp);

  lcd_chaine(buffer);
}

/**
 * Initialiser l'écran LCD
 */
int lcd_init(void) {
  int i, fd;
  struct lcdDataStruct *lcd;
  static int initialised = 0;
  unsigned char func;
  
  // Initialisation une seule fois du tableau lcds
  if (initialised == 0) {
    initialised = 1;
    for (i = 0; i < MAX_LCDS; ++i)
      lcds[i] = NULL;
  }

  // Configuration I2C
  fd = wiringPiI2CSetup(ADRESSE_LCD);
  if (fd < 0) {
    printf("Erreur : Initialisation I2C échouée\n");
    return -1;
  }
  
  // Configuration des registres MCP23008
  for (i = 0; i < 9; i++) {
    wiringPiI2CWriteReg8(fd, i, 0xFF);
  }
  
  // Configuration WiringPi
  wiringPiSetupSys();
  mcp23008Setup(AF_BASE, ADRESSE_LCD);
  pinMode(AF_BLUE, OUTPUT);

  // Recherche d'un emplacement libre dans le tableau lcds
  for (i = 0; i < MAX_LCDS; ++i) {
    if (lcds[i] == NULL) {
      lcdHandle = i;
      break;
    }
  }

  if (lcdHandle == -1)
    return -1;

  // Création d'une nouvelle structure LCD
  lcd = (struct lcdDataStruct *)malloc(sizeof(struct lcdDataStruct));
  if (lcd == NULL)
    return -1;

  // Paramètres de l'écran LCD 16x2 en mode 4 bits
  lcd->rsPin = AF_RS;
  lcd->strbPin = AF_STRB;
  lcd->bits = 8;  // Temporairement en 8 bits pour l'initialisation
  lcd->rows = 2;
  lcd->cols = 16;
  lcd->cx = 0;
  lcd->cy = 0;

  lcd->dataPins[0] = AF_DB4;
  lcd->dataPins[1] = AF_DB5;
  lcd->dataPins[2] = AF_DB6;
  lcd->dataPins[3] = AF_DB7;
  lcd->dataPins[4] = 0;
  lcd->dataPins[5] = 0;
  lcd->dataPins[6] = 0;
  lcd->dataPins[7] = 0;

  lcds[lcdHandle] = lcd;

  // Configuration des broches
  digitalWrite(lcd->rsPin, 0); pinMode(lcd->rsPin, OUTPUT);
  digitalWrite(lcd->strbPin, 0); pinMode(lcd->strbPin, OUTPUT);

  for (i = 0; i < 4; ++i) {
    digitalWrite(lcd->dataPins[i], 0);
    pinMode(lcd->dataPins[i], OUTPUT);
  }
  delay(35);  // ms

  // Initialisation en mode 4 bits
  func = LCD_FUNC | LCD_FUNC_DL;      // Set 8-bit mode 3 times
  put4Command(lcd, func >> 4); delay(35);
  put4Command(lcd, func >> 4); delay(35);
  put4Command(lcd, func >> 4); delay(35);
  func = LCD_FUNC;                    // 4th set: 4-bit mode
  put4Command(lcd, func >> 4); delay(35);
  lcd->bits = 4;

  // Configuration pour écran multi-lignes
  func |= LCD_FUNC_N;
  putCommand(lcd, func); delay(35);

  // Fin de la séquence d'initialisation
  lcd_affichage(TRUE);
  lcd_curseur(FALSE);
  lcd_curseur_clignote(FALSE);
  lcd_effacer();

  putCommand(lcd, LCD_ENTRY | LCD_ENTRY_ID);
  putCommand(lcd, LCD_CDSHIFT | LCD_CDSHIFT_RL);

  return lcdHandle;
}

/**
 * Écrire un message à une position spécifique
 */
void lcd_ecrire(int colonne, int ligne, char* message) {
  lcd_position(colonne, ligne);
  lcd_chaine(message);
}

/**
 * Libère les ressources de l'écran LCD
 */
void lcd_cleanup() {
    struct lcdDataStruct *lcd;
    
    // Vérification de la validité du descripteur
    if (lcdHandle < 0 || lcdHandle >= MAX_LCDS || lcds[lcdHandle] == NULL)
        return;
        
    lcd = lcds[lcdHandle];
    
    // Effacer l'écran
    lcd_effacer();
    
    // Éteindre l'affichage
    lcd_affichage(FALSE);
    
    // Configurer les broches en entrée pour sécuriser
    pinMode(lcd->rsPin, INPUT);
    pinMode(lcd->strbPin, INPUT);
    
    int i;
    for (i = 0; i < 4; ++i) {
        pinMode(lcd->dataPins[i], INPUT);
    }
    
    // Éteindre le rétroéclairage si disponible
    pinMode(AF_BLUE, INPUT);
    
    // Libérer la mémoire
    free(lcd);
    lcds[lcdHandle] = NULL;
}
