#include <stdio.h>
#include <wiringPi.h>
#include <signal.h>
#include "my_touch.h"

// Drapeau pour la boucle principale
int running = 1;

// Gestionnaire de signal pour arrêter proprement
void handle_signal(int sig)
{
    if (sig == SIGINT) {
        printf("\nInterruption détectée, arrêt du test...\n");
        running = 0;
    }
}

int main() {
    // Configuration du gestionnaire de signal
    signal(SIGINT, handle_signal);

    // Initialisation
    if (wiringPiSetup() == -1) {
        printf("Erreur d'initialisation de WiringPi\n");
        return 1;
    }

    touch_init();
    printf("Test du capteur tactile. Appuyez sur Ctrl+C pour quitter.\n");

    while (running) {
        int state = touch_read();
        printf("Touch state: %d\n", state);
        delay(500); // Attendre 500ms avant de lire à nouveau
    }
    
    // Nettoyage du capteur tactile
    touch_cleanup();
    printf("Test terminé.\n");
    
    return 0;
}
