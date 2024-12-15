#include "client_manager.h"

extern pthread_mutex_t clients_mutex; // Mutex pour synchroniser l'accès au tableau des clients
extern client_t clients[MAX_CLIENTS]; // Tableau des clients

int checked(int ret, char *calling_function)
{
    if (ret < 0)
    {
        perror(calling_function);
        exit(EXIT_FAILURE);
    }
    return ret;
}

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
    return -1; // Aucun emplacement libre
}

client_t *findClientByPseudo(const char *pseudo)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].is_active && strcmp(clients[i].pseudo, pseudo) == 0)
        {
            pthread_mutex_unlock(&clients_mutex);
            return &clients[i];
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return NULL; // Aucun client trouvé
}

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

void handle_client(int client_socket)
{
    char buffer[MAX_LEN_MESSAGE];
    ssize_t bytes_read;

    // Récupérer le pseudonyme du client
    bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0)
    {
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
    if (addClient(client_socket, client_address, pseudo) == -1)
    {
        printf("Impossible d'ajouter le client.\n");
        close(client_socket);
        return;
    }

    printf("Client connecté avec le pseudo : %s\n", pseudo);

    // Boucle principale pour gérer les messages
    while ((bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
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
        if (destinataire == NULL)
        {
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

void *client_thread(void *arg)
{
    int client_socket = *(int *)arg;
    free(arg); // Libérer la mémoire allouée pour le socket
    handle_client(client_socket);
    return NULL;
}
