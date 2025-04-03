#ifndef SERVEUR_H_INCLUDED
#define SERVEUR_H_INCLUDED

void attendre_clients(int server_socket);
void synchroniser_clients();
void envoyer_grille_et_forme(int forme);
void recevoir_temps(int *temps);
void envoyer_resultats(int meilleur_temps, int pire_temps);
int main(int argc, char *argv[]);

#endif // SERVEUR_H_INCLUDED
