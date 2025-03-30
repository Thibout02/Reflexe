#ifndef MY_NCURSES_H
#define MY_NCURSES_H

/**
 * Initialise la bibliothèque ncurses et configure l'écran
 */
void init_ncurses();

/**
 * Termine proprement l'utilisation de ncurses
 */
void cleanup_ncurses();

/**
 * Dessine la grille 4x4 au centre de l'écran
 */
void draw_grid();

/**
 * Affiche un motif 8x8 dans une cellule de la grille
 * @param row Ligne de la grille (0-3)
 * @param col Colonne de la grille (0-3)
 * @param pattern Tableau 8x8 de caractères ('x' pour dessiner, ' ' pour laisser vide)
 */
void display_pattern(int row, int col, char pattern[8][8]);

/**
 * Efface le contenu d'une cellule
 * @param row Ligne de la grille (0-3)
 * @param col Colonne de la grille (0-3)
 */
void clear_cell(int row, int col);

/**
 * Rafraîchit l'affichage
 */
void refresh_display();

#endif /* MY_NCURSES_H */
