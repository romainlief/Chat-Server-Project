#include "client_manager.h"

atomic_int client_count = 0;          // nombre de clients dans la file d'attente
extern pthread_mutex_t clients_mutex; // Mutex pour synchroniser l'accès au tableau des clients
extern client_t clients[MAX_CLIENTS]; // Tableau des clients

int addClient(int socket_fd, struct sockaddr_in address, const char *pseudo)
{
    checked(pthread_mutex_lock(&clients_mutex), "pthread_mutex_lock");

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (!clients[i].is_active)
        {
            clients[i].socket_fd = socket_fd;
            clients[i].address = address;
            clients[i].is_active = 1;
            strncpy(clients[i].pseudo, pseudo, MAX_LEN_PSEUDO - 1);
            clients[i].pseudo[MAX_LEN_PSEUDO - 1] = '\0';
            checked(pthread_mutex_unlock(&clients_mutex), "pthread_mutex_unlock");
            return 0;
        }
    }

    // Déverrouillage du mutex si aucun emplacement libre n'a été trouvé
    checked(pthread_mutex_unlock(&clients_mutex), "pthread_mutex_unlock");
    return -1; // Aucun emplacement libre
}

client_t *findClientByPseudo(const char *pseudo)
{
    checked(pthread_mutex_lock(&clients_mutex), "pthread_mutex_lock");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].is_active && strcmp(clients[i].pseudo, pseudo) == 0)
        {
            checked(pthread_mutex_unlock(&clients_mutex), "pthread_mutex_unlock");
            return &clients[i];
        }
    }
    checked(pthread_mutex_unlock(&clients_mutex), "pthread_mutex_unlock");
    return NULL; // Aucun client trouvé
}

int count_active_clients()
{
    int count = 0;
    checked(pthread_mutex_lock(&clients_mutex), "pthread_mutex_lock");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].is_active)
        {
            count++;
        }
    }
    checked(pthread_mutex_unlock(&clients_mutex), "pthread_mutex_unlock");
    return count;
}

void remove_client(int client_socket)
{
    checked(pthread_mutex_lock(&clients_mutex), "pthread_mutex_lock");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket_fd == client_socket && clients[i].is_active)
        {
            checked(close(client_socket), "close");
            clients[i].is_active = 0;
            break;
        }
    }
    checked(pthread_mutex_unlock(&clients_mutex), "pthread_mutex_unlock");
}

int add_client_with_pseudo(int client_socket, char *pseudo)
{
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    getpeername(client_socket, (struct sockaddr *)&client_address, &client_len);

    client_count++;
    while (addClient(client_socket, client_address, pseudo) == -1)
    {
        if (client_count >= MAX_CLIENTS)
        {
            close(client_socket);
            client_count--;
            return -1;
        }
        sleep(1);
    }
    client_count--;
    return 0;
}

void handle_client(int client_socket)
{
    char buffer[MAX_LEN_MESSAGE];
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    // Récupérer le pseudonyme du client
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

    if (add_client_with_pseudo(client_socket, pseudo) == -1)
    {
        return;
    }

    printf("Client connecté avec le pseudo : %s\n", pseudo);

    // Boucle principale pour gérer les messages
    while ((bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes_read] = '\0'; // Null-terminate

        if (bytes_read > MAX_LEN_MESSAGE - 1)
        {
            printf("Message trop long reçu de %s. Déconnexion du client.\n", pseudo);
            close(client_socket);
            remove_client(client_socket);
            return;
        }

        // Extraire le pseudonyme du destinataire et le message

        char *pseudo_receveur = strtok(buffer, " ");
        if (pseudo_receveur == NULL)
        {
            printf("Message invalide reçu de %s\n", pseudo);
            continue;
        }
        char *message = strtok(NULL, "\0");
        if (message == NULL)
        {
            printf("Message invalide reçu de %s\n", pseudo);
            remove_client(client_socket);
            continue;
        }
        printf("message : %s\n", message);

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
