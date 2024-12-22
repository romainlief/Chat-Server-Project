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
#include "lecture.h"
#include "process.h"


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

/**
 * @brief Definir les variables d'environnement
 * 
 * @param port_name Nom de la variable d'environnement du port
 * @param port_value Valeur de la variable d'environnement du port
 * @param IP_name Nom de la variable d'environnement de l'adresse IP
 * @param IP_value Valeur de la variable d'environnement de l'adresse IP
 */
void set_env_variables(const char *port_name, const char *port_value, const char *IP_name, const char *IP_value);

/**
 * @brief Configurer et connecter le socket
 * 
 * @param sock Socket du serveur
 * @param serv_addr Adresse du serveur
 * @param port_name Nom de la variable d'environnement du port
 * @param IP_name Nom de la variable d'environnement de l'adresse IP
 */
void setup_and_connect_socket(int *sock, struct sockaddr_in *serv_addr, const char *port_name, const char *IP_name);

/**
 * @brief Lancer le chat
 * 
 * @param sock Socket du serveur
 * @param username Nom d'utilisateur
 */
void start_chat(int sock, char *username);

/**
 * @brief Lancer le client
 * 
 * @param argc Nombre d'arguments
 * @param argv Arguments
 */
void start_client(int argc, char *argv[]);

#endif // SERVEUR_CONNEXION_H
