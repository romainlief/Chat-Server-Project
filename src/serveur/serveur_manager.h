#ifndef SERVEUR_MANAGER_H
#define SERVEUR_MANAGER_H

#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "constants.h"
#include "client_manager.h"

/**
 * @brief Récupère le port du serveur depuis la variable d'environnement PORT_SERVEUR
 *
 * @return port du serveur
 */
int getServeurPort();

/**
 * @brief Initialise le serveur
 *
 * @param port le port du serveur
 * @return int le descripteur du socket du serveur
 */
int initServeur(int port);

/**
 * @brief Fonction principale du serveur
 *
 * @param server_socket le descripteur du socket du serveur
 */
void run_server(int server_socket);

#endif // SERVEUR_MANAGER_H
