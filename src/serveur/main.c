#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ctype.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 2
#define MAX_LEN_MESSAGE 1024
#define PORT_PAR_DEFAULT 1234
#define MAX_CONNEXIONS 1
#define MAX_LEN_PSEUDO 30

typedef struct
{
    int socket_fd;              // Descripteur du socket du client
    struct sockaddr_in address; // Adresse du client
    pthread_t thread_id;        // Thread associé
    int is_active;              // Indique si le client est actif
    char pseudo[MAX_LEN_PSEUDO];            // Pseudonyme du client
} client_t;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex pour synchroniser l'accès au tableau des clients
int server_fd;                                             // Descripteur du socket du serveur
client_t clients[MAX_CLIENTS];                             // Tableau des clients

/**
 * @brief Fonction de gestion des erreurs
 *
 * @param ret la valeur de retour de la fonction à vérifier
 * @param calling_function le nom de la fonction appelante
 * @return ret la valeur de retour de la fonction à vérifier
 */
int checked(int ret, char *calling_function)
{
    if (ret < 0)
    {
        perror(calling_function);
        exit(EXIT_FAILURE);
    }
    return ret;
}

/**
 * @brief Récupère le port du serveur depuis la variable d'environnement PORT_SERVEUR
 *
 * @return port du serveur
 */
int getServeurPort()
{
    int port = PORT_PAR_DEFAULT;
    char *port_env = getenv("PORT_SERVEUR");

    if (port_env != NULL)
    {
        for (int i = 0; port_env[i] != '\0'; i++)
        {
            if (!isdigit(port_env[i]))
            {
                fprintf(stderr, "Port invalide dans PORT_SERVEUR\n");
                return port;
            }
        }

        int env_port = atoi(port_env);
        if (env_port > 0 && env_port <= 65535)
        {
            port = env_port;
        }
        else
        {
            fprintf(stderr, "Port hors plage (1-65535), utilisation du port par défaut (%d)\n", PORT_PAR_DEFAULT);
        }
    }

    return port;
}

/**
 * @brief Initialise le serveur
 *
 * @param port le port du serveur
 * @return int le descripteur du socket du serveur
 */
int initServeur(int port)
{
    int opt = 1;
    struct sockaddr_in address;

    // Création du socket
    server_fd = checked(socket(AF_INET, SOCK_STREAM, 0), "socket");

    // Configuration de l'option de réutilisation de l'adresse
    checked(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)), "setsockopt");

    // Configuration de l'adresse du serveur
    address.sin_family = AF_INET;         // Choix du protocole IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Accepter les connexions de n'importe quelle adresse
    address.sin_port = htons(port);       // Conversion du port en format réseau

    checked(bind(server_fd, (struct sockaddr *)&address, sizeof(address)), "bind"); // Liaison du socket à l'adresse et au port spécifiés
    checked(listen(server_fd, MAX_CONNEXIONS), "listen");                           // Mise en écoute du serveur

    printf("Serveur démarré sur le port %d\n", port);

    return server_fd;
}

/**
 * @brief Ajoute un client au tableau des clients
 *
 * @param socket_fd le descripteur du socket du client
 * @param address l'adresse du client
 * @return int 0 si l'ajout a réussi, -1 sinon
 */
int addClient(int socket_fd, struct sockaddr_in address, const char *pseudo)
{
    if (pthread_mutex_lock(&clients_mutex) != 0)
    {
        perror("pthread_mutex_lock");
        return -1;
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (!clients[i].is_active)
        {
            clients[i].socket_fd = socket_fd;
            clients[i].address = address;
            clients[i].is_active = 1;
            strncpy(clients[i].pseudo, pseudo, MAX_LEN_PSEUDO - 1);
            clients[i].pseudo[MAX_LEN_PSEUDO - 1] = '\0';
            if (pthread_mutex_unlock(&clients_mutex) != 0)
            {
                perror("pthread_mutex_unlock");
                return -1;
            }
            return 0;
        }
    }

    // Déverrouillage du mutex si aucun emplacement libre n'a été trouvé
    if (pthread_mutex_unlock(&clients_mutex) != 0)
    {
        perror("pthread_mutex_unlock");
        return -1;
    }
}

client_t *findClientByPseudo(const char *pseudo) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].is_active && strcmp(clients[i].pseudo, pseudo) == 0) {
            pthread_mutex_unlock(&clients_mutex);
            return &clients[i];
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return NULL; // Aucun client trouvé
}

/**
 * @brief Compte le nombre de clients actifs
 *
 * @return int le nombre de clients actifs
 */
int count_active_clients()
{
    int count = 0;
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].is_active)
        {
            count++;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return count;
}

/**
 * @brief Supprime un client du tableau des clients
 *
 * @param client_socket le descripteur du socket du client
 */
void remove_client(int client_socket)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket_fd == client_socket && clients[i].is_active)
        {
            close(client_socket);
            clients[i].is_active = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}


/**
 * @brief Gère les messages reçus d'un client
 *
 * @param client_socket le descripteur du socket du client
 */
void handle_client(int client_socket) {
    char buffer[MAX_LEN_MESSAGE];
    ssize_t bytes_read;

    // Récupérer le pseudonyme du client
    bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        printf("Erreur ou déconnexion du client lors de l'envoi du pseudonyme.\n");
        remove_client(client_socket);
        return;
    }
    buffer[bytes_read] = '\0'; // Null-terminate
    char pseudo[MAX_LEN_PSEUDO];
    strncpy(pseudo, buffer, MAX_LEN_PSEUDO - 1);
    pseudo[MAX_LEN_PSEUDO - 1] = '\0';

    // Ajouter le client avec son pseudonyme
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    getpeername(client_socket, (struct sockaddr *)&client_address, &client_len);
    if (addClient(client_socket, client_address, pseudo) == -1) {
        printf("Impossible d'ajouter le client.\n");
        close(client_socket);
        return;
    }

    printf("Client connecté avec le pseudo : %s\n", pseudo);

    // Boucle principale pour gérer les messages
    while ((bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate
        printf("Message reçu du client %s : %s\n", pseudo, buffer);

        // Extraire le pseudonyme du destinataire et le message
        char *pseudo_envoyeur = strtok(buffer, " ");
        printf("pseudo_envoyeur : %s\n", pseudo_envoyeur);

        char *pseudo_receveur = strtok(NULL, " ");
        printf("pseudo_receveur : %s\n", pseudo_receveur);

        char *message = strtok(NULL, "\0");

        

        // Trouver le client destinataire
        client_t *destinataire = findClientByPseudo(pseudo_receveur);
        if (destinataire == NULL) {
            char error_msg[MAX_LEN_MESSAGE];
            snprintf(error_msg, sizeof(error_msg), "Le client '%s' n'est pas connecté.\n", pseudo_receveur);
            send(client_socket, error_msg, strlen(error_msg), 0);
            continue;
        }

        // Envoyer le message au destinataire
        char full_message[MAX_LEN_MESSAGE];
        snprintf(full_message, sizeof(full_message), "[%s] %s", pseudo, message);
        send(destinataire->socket_fd, full_message, strlen(full_message), 0);
    }

    printf("Client %s déconnecté\n", pseudo);
    remove_client(client_socket);
}


/**
 * @brief Fonction exécutée par les threads clients
 *
 * @param arg le descripteur du socket du client
 * @return void*
 */
void *client_thread(void *arg)
{
    int client_socket = *(int *)arg;
    free(arg); // Libérer la mémoire allouée pour le socket
    handle_client(client_socket);
    return NULL;
}

/**
 * @brief Fonction principale du serveur
 *
 * @param server_socket le descripteur du socket du serveur
 */
void run_server(int server_socket)
{
    while (1)
    {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);

        if (count_active_clients() >= MAX_CLIENTS)
        {
            sleep(1); // Attendre avant de réessayer
            continue;
        }

        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);
        if (client_socket < 0)
        {
            perror("accept");
            continue;
        }

        char pseudo[MAX_LEN_PSEUDO] = "default_pseudo"; // Replace with actual pseudo logic
        if (addClient(client_socket, client_address, pseudo) == -1)
        {
            fprintf(stderr, "Erreur lors de l'ajout du client\n");
            close(client_socket);
        }

        printf("Nouveau client connecté : %s:%d\n",
               inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Créer un thread pour gérer le client
        pthread_t thread_id;
        int *socket_ptr = malloc(sizeof(int));
        if (socket_ptr == NULL)
        {
            perror("malloc");
            close(client_socket);
            continue;
        }
        *socket_ptr = client_socket;

        if (pthread_create(&thread_id, NULL, client_thread, socket_ptr) != 0)
        {
            perror("pthread_create");
            free(socket_ptr);
            close(client_socket);
            continue;
        }

        // Détacher le thread pour libérer les ressources automatiquement
        pthread_detach(thread_id);
    }
}

int main(void)
{
    int port = getServeurPort();
    int server_socket = initServeur(port);

    run_server(server_socket);

    close(server_socket);

    return 0;
}
