#ifndef JEU_H_INCLUDED
#define JEU_H_INCLUDED

// Fonctions exposées pour le jeu
void initialiser_peripheriques();
double attendre_contact_tactile();
int jouer_partie(int *temps_final, int *position_correcte);
void afficher_resultat(int rep, int position_correcte, int temps_final);
void nettoyer_peripheriques();
int afficher_grille(int forme);
void convertir_forme(const int forme[8], char resultat[8][8]);

// Fonction principale du jeu
int lancer_jeu(void);

// Définition des formes
extern int formes[16][8];

#endif // JEU_H_INCLUDED
