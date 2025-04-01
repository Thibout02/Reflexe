#ifndef JEU_H_INCLUDED
#define JEU_H_INCLUDED
#include <stddef.h>

// Fonctions exposées pour le jeu
int initialiser_peripheriques();
double attendre_contact_tactile();
int jouer_partie(int *temps_final, int *position_correcte);
void afficher_resultat(int rep, int position_correcte, int temps_final);
void nettoyer_peripheriques();
int afficher_grille(int forme);
void convertir_forme(const int forme[8], char resultat[8][8]);

// Fonctions pour la communication réseau
int envoyer_donnees(int socket, const void *buffer, size_t length);
int recevoir_donnees(int socket, void *buffer, size_t length);

// Fonction principale du jeu
int lancer_jeu(void);

#endif // JEU_H_INCLUDED
