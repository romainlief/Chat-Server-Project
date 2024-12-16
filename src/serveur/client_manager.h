#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "constants.h"
#include "error_handler.h"

typedef struct
{
    int socket_fd;               // Descripteur du socket du client
    struct sockaddr_in address;  // Adresse du client
    pthread_t thread_id;         // Thread associé
    int is_active;               // Indique si le client est actif
    char pseudo[MAX_LEN_PSEUDO]; // Pseudonyme du client
} client_t;


/**
 * @brief Ajoute un client au tableau des clients
 *
 * @param socket_fd le descripteur du socket du client
 * @param address l'adresse du client
 * @param pseudo le pseudonyme du client
 * @return int 0 si l'ajout a réussi, -1 sinon
 */
int addClient(int socket_fd, struct sockaddr_in address, const char *pseudo);

/**
 * @brief Trouve un client par son pseudonyme
 *
 * @param pseudo le pseudonyme du client
 * @return client_t* le client trouvé, NULL sinon
 */
client_t *findClientByPseudo(const char *pseudo);

/**
 * @brief Compte le nombre de clients actifs
 *
 * @return int le nombre de clients actifs
 */
int count_active_clients();

/**
 * @brief Supprime un client du tableau des clients
 *
 * @param client_socket le descripteur du socket du client
 */
void remove_client(int client_socket);

/**
 * @brief Gère les messages reçus d'un client
 *
 * @param client_socket le descripteur du socket du client
 */
void handle_client(int client_socket);

/**
 * @brief Fonction exécutée par les threads clients
 *
 * @param arg le descripteur du socket du client
 * @return void*
 */
void *client_thread(void *arg);

#endif // CLIENT_MANAGER_H
