#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

// Prototypes des fonctions
int afficher_grille(int forme);
void convertir_forme(const int forme[8], char resultat[8][8]);
void initialiser_peripheriques();
double attendre_contact_tactile();
int jouer_partie(int *temps_final, int *position_correcte);
void afficher_resultat(int rep, int position_correcte, int temps_final);
void nettoyer_peripheriques();

// Définition des formes
extern int formes[16][8];

// Tableau externe défini dans un fichier .c
#endif // MAIN_H_INCLUDED
