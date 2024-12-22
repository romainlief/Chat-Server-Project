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
            checked(close(client_socket), "close");
            client_count--;
            return -1;
        }
        sleep(1);
    }
    client_count--;
    return 0;
}

void handle_message(char *buffer, const char *pseudo, int client_socket, ssize_t size_message)
{
    // Extraire le pseudonyme du destinataire et le message
    char *pseudo_receveur = strtok(buffer, " ");
    if (pseudo_receveur == NULL)
    {
        printf("Message invalide reçu de %s\n", pseudo);
        return;
    }
    char *message = strtok(NULL, "");
    if (message == NULL)
    {
        printf("Message invalide reçu de %s\n", pseudo);
        remove_client(client_socket);
        return;
    }
    char message2[size_message];
    strcpy(message2, message);

    // Trouver le client destinataire
    client_t *destinataire = findClientByPseudo(pseudo_receveur);
    if (destinataire == NULL)
    {
        char error_msg[MAX_LEN_MESSAGE + 1];
        snprintf(error_msg, sizeof(error_msg), "[/serveur/] Cette personne (%s) n'est pas connectée.\n", pseudo_receveur);
        checked((int)send(client_socket, error_msg, strlen(error_msg), 0), "send");
        return;
    }

    // Envoyer le message au destinataire
    char full_message[MAX_LEN_MESSAGE + 1];
    snprintf(full_message, sizeof(full_message), "[%s] %s", pseudo, message2);    
    checked((int)send(destinataire->socket_fd, full_message, strlen(full_message), 0), "send");
    memset(message2, 0, sizeof(message2));

}

int handle_pseudo(int client_socket, char *pseudo)
{
    char buffer[MAX_LEN_MESSAGE + 1];
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read <= 0)
    {
        printf("Erreur ou déconnexion du client lors de l'envoi du pseudonyme.\n");
        remove_client(client_socket);
        return -1;
    }
    strncpy(pseudo, buffer, MAX_LEN_PSEUDO - 1);

    if (add_client_with_pseudo(client_socket, pseudo) == -1)
    {
        return -1;
    }

    printf("Client connecté avec le pseudo : %s\n", pseudo);
    return 0;
}

void main_message_loop(int client_socket, const char *pseudo)
{
    char buffer[MAX_LEN_MESSAGE + 1];
    ssize_t bytes_read;
    
    while ((bytes_read = recv(client_socket, buffer, sizeof(buffer), 0)) > 0)
    {
        if (bytes_read > MAX_LEN_MESSAGE + 1)
        {
            printf("Message trop long reçu de %s. Déconnexion du client.\n", pseudo);
            remove_client(client_socket);
            return;
        }

        handle_message(buffer, pseudo, client_socket, bytes_read);
        memset(buffer, 0, sizeof(buffer));
    }

    printf("Client %s déconnecté\n", pseudo);
    remove_client(client_socket);
}

void handle_client(int client_socket)
{
    char pseudo[MAX_LEN_PSEUDO];

    if (handle_pseudo(client_socket, pseudo) == -1)
    {
        return;
    }
    main_message_loop(client_socket, pseudo);
}

void *client_thread(void *arg)
{
    int client_socket = *(int *)arg;
    free(arg); // Libérer la mémoire allouée pour le socket
    handle_client(client_socket);
    return NULL;
}
