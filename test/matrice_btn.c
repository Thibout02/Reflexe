#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "my_matrice_btn.h"

#include <wiringPi.h>

// Fonction pour gérer l'interruption
int running = 1;
void handle_sigint(int sig) {
    if (sig == SIGINT)
    {
        running = 0;
        printf("\nInterruption détectée, arrêt du test...\n");
    }
}

// Prototypes
void clear_screen(void);
void afficher_matrice(int bouton_presse);

int main()
{
    // Configuration du gestionnaire de signal pour CTRL+C
    signal(SIGINT, handle_sigint);
    
    printf("=== Test de la matrice de boutons ===\n");
    
    // Initialisation
    if (wiringPiSetup() == -1) {
        printf("Erreur d'initialisation de WiringPi\n");
        return 1;
    }
    
    matrice_btn_init();
    printf("Initialisation terminée\n\n");
    
    clear_screen();
    printf("Appuyez sur des boutons. Appuyez sur Ctrl+C pour terminer.\n\n");
    afficher_matrice(0);

    int dernier_bouton = 0;

    while (running) {
        int bouton = matrice_btn_lire();

        if (bouton != dernier_bouton) {
            dernier_bouton = bouton;

            clear_screen();
            printf("=== Test de la matrice de boutons ===\n");
            printf("Appuyez sur des boutons. Appuyez sur Ctrl+C pour terminer.\n\n");

            afficher_matrice(bouton);

            if (bouton != 0) {
                int ligne, colonne;
                matrice_btn_obtenir_position(bouton, &ligne, &colonne);
                printf("\nBouton pressé: %d, Ligne: %d, Colonne: %d\n", bouton, ligne, colonne);
            } else {
                printf("\nAucun bouton pressé\n");
            }
        }

        delay(50); // Réduit pour une meilleure réactivité
    }

    printf("Test terminé\n");

    // Nettoyage de la matrice de boutons
    matrice_btn_cleanup();

    return 0;
}

void clear_screen(void) {
    // Commande ANSI pour effacer l'écran et positionner le curseur en haut
    printf("\033[2J\033[H");
}

void afficher_matrice(int bouton_presse)
{
    int ligne, col;

    printf("┌────┬────┬────┬────┐\n");
    for (ligne = 0; ligne < 4; ligne++) {
        printf("│");
        for (col = 0; col < 4; col++) {
            int btn = ligne * 4 + col + 1;
            if (btn == bouton_presse) {
                printf(" \033[1;32mX\033[0m  │"); // Bouton pressé en vert et gras
            } else {
                printf(" %-2d │", btn); // Format fixed-width pour assurer l'alignement
            }
        }
        
        if (ligne < 3) {
            printf("\n├────┼────┼────┼────┤\n");
        } else {
            printf("\n└────┴────┴────┴────┘\n");
        }
    }
}
