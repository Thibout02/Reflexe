#ifndef MY_TOUCH_H
#define MY_TOUCH_H

void touch_init();              // Initialiser le capteur de touch
int touch_read();               // Lire l'état du capteur (1 quand touché, 0 sinon)
void touch_cleanup();           // Nettoyer et réinitialiser la broche du capteur

#endif // MY_TOUCH_H
