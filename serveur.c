#include "jeu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdbool.h>

#define DEFAULT_PORT 12345
#define MAX_CLIENTS 10
#define DELAI_CONNEXION 5 // Délai d'attente en secondes pour d'autres connexions

int clients[MAX_CLIENTS];
int nb_clients = 0;
bool clients_actifs[MAX_CLIENTS];

void attendre_clients(int server_socket) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    printf("En attente de connexions des clients...\n");
    
    // Attendre au moins une connexion
    while (nb_clients == 0) {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket >= 0) {
            clients[nb_clients] = client_socket;
            clients_actifs[nb_clients] = true;
            nb_clients++;
            printf("Client %d connecté. (%d/%d)\n", nb_clients, nb_clients, MAX_CLIENTS);
        }
    }
    
    // Une fois qu'au moins un client est connecté, continuer à accepter les connexions
    // pendant un certain temps pour donner la chance à d'autres joueurs de se connecter
    printf("Premier joueur connecté, attente de %d secondes pour d'autres joueurs...\n", DELAI_CONNEXION);
    
    // Configuration du timeout
    fd_set read_fds;
    struct timeval timeout;
    timeout.tv_sec = DELAI_CONNEXION;
    timeout.tv_usec = 0;
    
    while (nb_clients < MAX_CLIENTS) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        
        // Vérifier si une nouvelle connexion arrive pendant le délai
        int activity = select(server_socket + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity <= 0) {
            // Timeout atteint ou erreur
            break;
        }
        
        if (FD_ISSET(server_socket, &read_fds)) {
            int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
            if (client_socket >= 0) {
                clients[nb_clients] = client_socket;
                clients_actifs[nb_clients] = true;
                nb_clients++;
                printf("Client %d connecté. (%d/%d)\n", nb_clients, nb_clients, MAX_CLIENTS);
                
                // Réinitialiser le délai après chaque nouvelle connexion
                timeout.tv_sec = DELAI_CONNEXION;
                timeout.tv_usec = 0;
            }
        }
    }
    
    printf("Démarrage du jeu avec %d joueurs. Les joueurs doivent appuyer sur le bouton tactile pour commencer.\n", nb_clients);
}

bool attendre_pression_touch(int server_socket) {
    printf("Attente que tous les joueurs maintiennent le bouton tactile...\n");
    printf("D'autres joueurs peuvent se connecter pendant cette attente.\n");
    
    // Tableau pour suivre les clients qui ont déjà envoyé leur confirmation
    bool client_pret[MAX_CLIENTS] = {false};
    char buffer[1];
    
    fd_set read_fds;
    struct timeval timeout;
    int max_fd;
    
    while (true) {
        FD_ZERO(&read_fds);
        
        // Ajouter le socket serveur à l'ensemble pour accepter de nouvelles connexions
        FD_SET(server_socket, &read_fds);
        max_fd = server_socket;
        
        // Ajouter tous les clients actifs à l'ensemble
        int i;
        for (i = 0; i < nb_clients; i++) {
            if (clients_actifs[i]) {
                FD_SET(clients[i], &read_fds);
                if (clients[i] > max_fd) {
                    max_fd = clients[i];
                }
            }
        }
        
        // Définir un timeout court
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000; // 500ms
        
        // Attendre les événements sur les sockets
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity < 0) {
            perror("Erreur lors de select");
            return false;
        }
        
        // Vérifier si une nouvelle connexion arrive
        if (FD_ISSET(server_socket, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            
            int new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
            if (new_socket >= 0) {
                // Ajouter le nouveau client s'il y a de la place
                if (nb_clients < MAX_CLIENTS) {
                    clients[nb_clients] = new_socket;
                    clients_actifs[nb_clients] = true;
                    client_pret[nb_clients] = false;
                    nb_clients++;
                    printf("Nouveau client %d connecté pendant l'attente. (%d/%d)\n", 
                           nb_clients, nb_clients, MAX_CLIENTS);
                } else {
                    printf("Nombre maximum de clients atteint, connexion refusée.\n");
                    close(new_socket);
                }
            }
        }
        
        // Vérifier les messages des clients
        for (i = 0; i < nb_clients; i++) {
            if (clients_actifs[i] && !client_pret[i] && FD_ISSET(clients[i], &read_fds)) {
                int res = recevoir_donnees(clients[i], buffer, 1);
                if (res <= 0) {
                    // Le client s'est déconnecté
                    printf("Client %d déconnecté pendant l'attente.\n", i+1);
                    clients_actifs[i] = false;
                    close(clients[i]);
                } else if (buffer[0] == '1') {
                    // Le client est prêt
                    client_pret[i] = true;
                    printf("Client %d est prêt (bouton tactile maintenu).\n", i+1);
                }
            }
        }
        
        // Vérifier si tous les clients actifs sont prêts
        int tous_prets = 0;
        int clients_actifs_total = 0;
        
        for (i = 0; i < nb_clients; i++) {
            if (clients_actifs[i]) {
                clients_actifs_total++;
                if (client_pret[i]) {
                    tous_prets++;
                }
            }
        }
        
        // Si aucun client n'est actif, retourner false
        if (clients_actifs_total == 0) {
            printf("Tous les clients sont déconnectés.\n");
            return false;
        }
        
        // Si tous les clients actifs sont prêts, on peut commencer le jeu
        if (tous_prets == clients_actifs_total && clients_actifs_total > 0) {
            break;
        }
    }
    
    printf("Tous les joueurs sont prêts! (%d joueurs)\n", nb_clients);
    
    // Envoyer un signal à tous les clients pour démarrer la partie
    char signal = '1';
    int i;
    for (i = 0; i < nb_clients; i++) {
        if (clients_actifs[i]) {
            envoyer_donnees(clients[i], &signal, 1);
        }
    }
    
    return true;
}

void envoyer_grille_et_forme() {
    // Générer une forme aléatoire à envoyer à tous les clients
    srand(time(NULL));
    int forme = rand() % 16;
    
    // Créer un tableau d'indices (0-15) qui sera le même pour tous les clients
    int indices[16];
    int i, j, temp;

    // Initialisation du tableau d'indices
    for (i = 0; i < 16; i++) {
        indices[i] = i;
    }

    // Mélange aléatoire des indices (Fisher-Yates shuffle)
    for (i = 15; i > 0; i--) {
        j = rand() % (i + 1);
        // Échange des indices i et j
        temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    printf("Envoi de la forme %d et de l'ordre des indices à tous les clients...\n", forme);

    // Envoyer la même forme et le même ordre d'indices à tous les clients
    for (i = 0; i < nb_clients; i++) {
        if (clients_actifs[i]) {
            // Envoyer d'abord la forme à chercher
            envoyer_donnees(clients[i], &forme, sizeof(int));

            // Puis envoyer l'ordre des indices (la disposition des formes sur la grille)
            envoyer_donnees(clients[i], indices, sizeof(indices));
        }
    }
}

void recevoir_temps(int *temps, bool *reponses_correctes) {
    printf("Réception des réponses et temps des clients...\n");

    // Initialiser tous les temps à la valeur maximale (considérés comme lents)
    int i;
    for (i = 0; i < nb_clients; i++) {
        temps[i] = 100; // 10 secondes (temps maximum)
        reponses_correctes[i] = false;
    }

    // Recevoir d'abord si la réponse est correcte, puis le temps de tous les clients actifs
    for (i = 0; i < nb_clients; i++) {
        if (clients_actifs[i]) {
            int est_correct = 0;
            int res = recevoir_donnees(clients[i], &est_correct, sizeof(int));

            if (res <= 0) {
                // Le client s'est déconnecté
                printf("Client %d déconnecté pendant la réception des résultats.\n", i+1);
                clients_actifs[i] = false;
                close(clients[i]);
                continue;
            }

            reponses_correctes[i] = (est_correct == 1);

            res = recevoir_donnees(clients[i], &temps[i], sizeof(int));
            if (res <= 0) {
                // Le client s'est déconnecté
                printf("Client %d déconnecté pendant la réception des temps.\n", i+1);
                clients_actifs[i] = false;
                close(clients[i]);
                temps[i] = 100; // Considéré comme lent
                reponses_correctes[i] = false;
            } else {
                // Pour les réponses incorrectes, on garde le temps mais on l'indique
                printf("Client %d: réponse %s, temps = %d.%d secondes\n",
                       i+1,
                       reponses_correctes[i] ? "correcte" : "incorrecte",
                       temps[i]/10, temps[i]%10);
            }
        }
    }
}

void envoyer_resultats(int *temps, bool *reponses_correctes) {
    // Calculer le meilleur et le pire temps parmi tous les clients actifs
    int meilleur_temps = 100; // Initialiser au maximum (10 secondes)
    int pire_temps = 0;       // Initialiser au minimum

    int i;
    int clients_actifs_total = 0;

    // Trouver le meilleur temps (réponses correctes uniquement)
    // et le pire temps (toutes réponses confondues)
    for (i = 0; i < nb_clients; i++) {
        if (clients_actifs[i]) {
            clients_actifs_total++;

            // Pour le meilleur temps, considérer uniquement les réponses correctes
            if (reponses_correctes[i] && temps[i] < meilleur_temps) {
                meilleur_temps = temps[i];
            }

            // Pour le pire temps, prendre le plus grand (même pour les réponses incorrectes)
            if (temps[i] > pire_temps) {
                pire_temps = temps[i];
            }
        }
    }

    // Si aucun client n'a donné de réponse correcte
    if (meilleur_temps == 100 && clients_actifs_total > 0) {
        printf("Aucune réponse correcte.\n");
    }

    printf("Meilleur temps (réponses correctes): %d.%d secondes\n", meilleur_temps/10, meilleur_temps%10);
    printf("Pire temps (toutes réponses): %d.%d secondes\n", pire_temps/10, pire_temps%10);

    // Envoyer les résultats à tous les clients actifs
    for (i = 0; i < nb_clients; i++) {
        if (clients_actifs[i]) {
            envoyer_donnees(clients[i], &meilleur_temps, sizeof(int));
            envoyer_donnees(clients[i], &pire_temps, sizeof(int));
        }
    }
}

bool recevoir_choix_rejouer() {
    printf("Attente des choix des joueurs pour rejouer...\n");

    char choix[MAX_CLIENTS];
    int rejouer_count = 0;
    int actifs_count = 0;

    // Compter les clients actifs
    int i;
    for (i = 0; i < nb_clients; i++) {
        if (clients_actifs[i]) {
            actifs_count++;
        }
    }

    if (actifs_count == 0) {
        return false; // Plus de clients actifs
    }

    // Recevoir les choix de tous les clients actifs
    for (i = 0; i < nb_clients; i++) {
        if (clients_actifs[i]) {
            int res = recevoir_donnees(clients[i], &choix[i], 1);
            if (res <= 0) {
                // Le client s'est déconnecté
                printf("Client %d déconnecté pendant la réception des choix.\n", i+1);
                clients_actifs[i] = false;
                close(clients[i]);
            } else if (choix[i] == 'o') {
                rejouer_count++;
                printf("Client %d veut rejouer.\n", i+1);
            } else {
                printf("Client %d ne veut pas rejouer.\n", i+1);
                clients_actifs[i] = false;
                close(clients[i]);
            }
        }
    }

    // Si au moins un client veut rejouer
    return rejouer_count > 0;
}

int main(int argc, char *argv[]) {
    // Traitement des arguments de la ligne de commande
    int port = DEFAULT_PORT;
    // Vérifier si le port a été fourni
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Port invalide, utilisation du port par défaut: %d\n", DEFAULT_PORT);
            port = DEFAULT_PORT;
        }
    }

    printf("Démarrage du serveur sur le port %d\n", port);

    // Initialisation du socket serveur
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Erreur lors de la création du socket");
        return EXIT_FAILURE;
    }

    // Configurer l'option de réutilisation d'adresse
    int option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    // Configuration de l'adresse du serveur
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind et listen
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur lors du bind");
        close(server_socket);
        return EXIT_FAILURE;
    }

    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Erreur lors du listen");
        close(server_socket);
        return EXIT_FAILURE;
    }

    printf("Serveur démarré sur le port %d\n", port);

    // Attendre les clients
    attendre_clients(server_socket);

    // Boucle principale du jeu
    bool continuer = true;
    while (continuer) {
        // Attendre que tous les clients maintiennent le bouton tactile
        if (!attendre_pression_touch(server_socket)) {
            printf("Plus de clients actifs, fin du jeu.\n");
            break;
        }

        // Envoyer la grille et la forme à tous les clients
        envoyer_grille_et_forme();

        // Recevoir les réponses et temps des clients
        int temps[MAX_CLIENTS];
        bool reponses_correctes[MAX_CLIENTS];
        recevoir_temps(temps, reponses_correctes);

        // Envoyer les résultats aux clients (meilleur/pire temps parmi les réponses correctes)
        envoyer_resultats(temps, reponses_correctes);

        // Demander aux clients s'ils veulent rejouer
        continuer = recevoir_choix_rejouer();

        // Si plus aucun client ne veut jouer, on quitte
        if (!continuer) {
            printf("Aucun client ne veut rejouer, fin du jeu.\n");
            break;
        }
    }

    // Fermer tous les sockets
    int i;
    for (i = 0; i < nb_clients; i++) {
        if (clients_actifs[i]) {
            close(clients[i]);
        }
    }

    close(server_socket);
    printf("Serveur arrêté.\n");

    return EXIT_SUCCESS;
}
