#include "my_ncurses.h"
#include <ncurses/ncurses.h>

// Dimensions de la grille
#define GRID_SIZE 4
#define CELL_WIDTH 8
#define CELL_HEIGHT 8

// Variables globales pour stocker les dimensions et position
int screen_height, screen_width;
int start_x, start_y;

void init_ncurses() {
    initscr();            // Initialise l'écran
    cbreak();             // Désactive la mise en buffer de ligne
    noecho();             // N'affiche pas les caractères tapés
    keypad(stdscr, TRUE); // Active les touches spéciales
    curs_set(0);          // Cache le curseur

    // Récupère les dimensions de l'écran
    getmaxyx(stdscr, screen_height, screen_width);

    // Calcule la position de départ pour centrer la grille
    // Une case fait CELL_WIDTH caractères de large (+ 4 caractères pour les bordures "────")
    int grid_width = (CELL_WIDTH + 1) * GRID_SIZE + 1;
    int grid_height = (CELL_HEIGHT + 1) * GRID_SIZE + 1;

    start_x = (screen_width - grid_width) / 2;
    start_y = (screen_height - grid_height) / 2;

    clear();
}

void cleanup_ncurses() {
    endwin();
}

void draw_grid() {
    int i, j, k;

    // Dessine la première ligne avec les coins supérieurs
    mvaddch(start_y, start_x, ACS_ULCORNER);  // ┌
    for (i = 0; i < GRID_SIZE; i++) {
        for (j = 0; j < CELL_WIDTH; j++) {
            mvaddch(start_y, start_x + 1 + i * (CELL_WIDTH + 1) + j, ACS_HLINE);  // ─
        }
        if (i < GRID_SIZE - 1)
            mvaddch(start_y, start_x + (i + 1) * (CELL_WIDTH + 1), ACS_TTEE);  // ┬
    }
    mvaddch(start_y, start_x + GRID_SIZE * (CELL_WIDTH + 1), ACS_URCORNER);  // ┐

    // Dessine les lignes intermédiaires
    for (i = 0; i < GRID_SIZE - 1; i++) {
        // Intersection gauche
        mvaddch(start_y + (i + 1) * (CELL_HEIGHT + 1), start_x, ACS_LTEE);  // ├

        // Lignes horizontales et intersections
        for (j = 0; j < GRID_SIZE; j++) {
            // Lignes horizontales (────)
            for (k = 0; k < CELL_WIDTH; k++) {
                mvaddch(start_y + (i + 1) * (CELL_HEIGHT + 1),
                       start_x + 1 + j * (CELL_WIDTH + 1) + k, ACS_HLINE);  // ─
            }

            // Croisements internes (┼)
            if (j < GRID_SIZE - 1) {
                mvaddch(start_y + (i + 1) * (CELL_HEIGHT + 1),
                       start_x + (j + 1) * (CELL_WIDTH + 1), ACS_PLUS);  // ┼
            }
        }

        // Intersection droite
        mvaddch(start_y + (i + 1) * (CELL_HEIGHT + 1),
               start_x + GRID_SIZE * (CELL_WIDTH + 1), ACS_RTEE);  // ┤
    }

    // Dessine la dernière ligne avec les coins inférieurs
    mvaddch(start_y + GRID_SIZE * (CELL_HEIGHT + 1), start_x, ACS_LLCORNER);  // └
    for (i = 0; i < GRID_SIZE; i++) {
        for (j = 0; j < CELL_WIDTH; j++) {
            mvaddch(start_y + GRID_SIZE * (CELL_HEIGHT + 1),
                   start_x + 1 + i * (CELL_WIDTH + 1) + j, ACS_HLINE);  // ─
        }
        if (i < GRID_SIZE - 1)
            mvaddch(start_y + GRID_SIZE * (CELL_HEIGHT + 1),
                   start_x + (i + 1) * (CELL_WIDTH + 1), ACS_BTEE);  // ┴
    }
    mvaddch(start_y + GRID_SIZE * (CELL_HEIGHT + 1),
           start_x + GRID_SIZE * (CELL_WIDTH + 1), ACS_LRCORNER);  // ┘

    // Dessine les lignes verticales pour compléter la grille
    int row;
    for (i = 0; i <= GRID_SIZE; i++) {
        for (j = 1; j <= CELL_HEIGHT; j++) {
            for (row = 0; row < GRID_SIZE; row++) {
                int y_pos = start_y + row * (CELL_HEIGHT + 1) + j;
                int x_pos = start_x + i * (CELL_WIDTH + 1);
                mvaddch(y_pos, x_pos, ACS_VLINE);  // │
            }
        }
    }

    refresh();
}

void display_pattern(int row, int col, char pattern[8][8]) {
    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
        return; // Vérification des limites
    }

    // Calcule la position de la cellule
    int cell_start_y = start_y + row * (CELL_HEIGHT + 1) + 1;
    int cell_start_x = start_x + col * (CELL_WIDTH + 1) + 1;

    // Affiche le motif
    int i, j;
    for (i = 0; i < CELL_HEIGHT; i++) {
        for (j = 0; j < CELL_WIDTH; j++) {
            mvaddch(cell_start_y + i, cell_start_x + j, pattern[i][j]);
        }
    }
}

void clear_cell(int row, int col) {
    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
        return; // Vérification des limites
    }

    // Calcule la position de la cellule
    int cell_start_y = start_y + row * (CELL_HEIGHT + 1) + 1;
    int cell_start_x = start_x + col * (CELL_WIDTH + 1) + 1;

    // Rempli la cellule avec des espaces
    int i, j;
    for (i = 0; i < CELL_HEIGHT; i++) {
        for (j = 0; j < CELL_WIDTH; j++) {
            mvaddch(cell_start_y + i, cell_start_x + j, ' ');
        }
    }
}

void refresh_display() {
    refresh();
}
