/**
 * Bibliothèque pour Afficheur à 7 Segments
 */
#ifndef SEGMENT7_H
#define SEGMENT7_H

// Adresse du périphérique et constantes
#define HT16K33_ADDRESS 0x70
#define MAX_DIGITS 4


// Initialiser l'afficheur
void segment7_init(void);

// Afficher un nombre
void segment7_afficher_nombre(int nombre);

// Effacer l'afficheur
void segment7_effacer(void);

// Régler la luminosité (0-15)
void segment7_luminosite(int luminosite);

// Écrire un chiffre à la position (0-3)
void segment7_ecrire_chiffre(int position, int valeur);

// Activer/désactiver le point décimal
void segment7_decimal(int position, int actif);

// Activer/désactiver les deux-points
void segment7_deuxpoints(int actif);

// Nettoie et libère les ressources de l'afficheur 7 segments
void segment7_cleanup(void);

#endif /* SEGMENT7_H */
