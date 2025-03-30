#include "my_touch.h"
#include <wiringPi.h>

/// Définition du pin pour le capteur de touch
#define TOUCH_PIN 0

void touch_init() {
    pinMode(TOUCH_PIN, INPUT);              // Configurer le pin comme entrée
    pullUpDnControl(TOUCH_PIN, PUD_UP);     // Activer la résistance de tirage vers le haut
    delay(100);                             // Délai de stabilisation
}

int touch_read() {
    // Avec une résistance de pull-up, le pin est LOW quand il n'est pas touché
    // et HIGH quand il est touché
    return digitalRead(TOUCH_PIN) == HIGH;
}

void touch_cleanup() {
    // Réinitialiser la broche en mode INPUT (état par défaut)
    pinMode(TOUCH_PIN, INPUT);
    // Désactiver les résistances de tirage
    pullUpDnControl(TOUCH_PIN, PUD_OFF);
}
