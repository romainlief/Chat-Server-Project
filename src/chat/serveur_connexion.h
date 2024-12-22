#ifndef SERVEUR_CONNEXION_H
#define SERVEUR_CONNEXION_H

#include "constants.h"

#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# include <sys/types.h>
# include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include "parametres.h"

#include "struct.h"

/**
 * @brief Verfier que l'adresse IP a une forme valide
 * 
 * @return 1 si valide, 0 si non
 */
int checkIP();

/**
 * @brief Verifie que le port est un nombre valide
 * 
 * @return 1 si valide, 0 si non
 */
int checkPort();

/**
 * @brief Initialise la connexion du serveur
 * 
 * @param sock Socket du serveur
 * @param serv_addr Adresse du serveur
 * @param port_name Nom de la variable d'environnement du port
 * @param IP_name Nom de la variable d'environnement de l'adresse IP
 */
void init_chat(int *sock, struct sockaddr_in *serv_addr, const char *port_name, const char *IP_name);

#endif // SERVEUR_CONNEXION_H
