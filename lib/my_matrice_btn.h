/**
 * Bibliothèque Matrice de Boutons
 */
#ifndef MATRICE_BTN_H
#define MATRICE_BTN_H

// Définitions des broches de la matrice de boutons
#define ROW1 2
#define ROW2 3
#define ROW3 21
#define ROW4 22

#define COL1 23
#define COL2 24
#define COL3 25
#define COL4 6

// Initialiser les broches de la matrice de boutons
void matrice_btn_init(void);

// Lit la matrice de boutons, renvoie le numéro du bouton (1-16) ou 0 si aucun n'est pressé
int matrice_btn_lire(void);

// Convertit le numéro du bouton en ligne et colonne
void matrice_btn_obtenir_position(int bouton, int *ligne, int *colonne);

// Attente d'appui sur un bouton avec temporisation
int matrice_btn_attendre_appui_timeout(int timeout_ms);

// Attente d'appui sur un bouton sans temporisation
int matrice_btn_attendre_appui(void);

// Attente de relâchement du bouton avec temporisation
int matrice_btn_attendre_relache_timeout(int timeout_ms);

// Attente de relâchement du bouton
int matrice_btn_attendre_relache(void);

// Nettoie et libère les ressources de la matrice de boutons
void matrice_btn_cleanup(void);

#endif /* MATRICE_BTN_H */
