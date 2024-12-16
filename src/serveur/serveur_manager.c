#include "serveur_manager.h"

int server_fd; // Descripteur du socket du serveur

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
        if (env_port >= BORNES_PORT_MIN && env_port <= BORNES_PORT_MAX)
        {
            port = env_port;
        }
        else
        {
            fprintf(stderr, "Port hors bornes (1-65535), utilisation du port par défaut (%d)\n", PORT_PAR_DEFAULT);
        }
    }

    return port;
}

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

void run_server(int server_socket)
{
    while (1)
    {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);

        // Accepter une nouvelle connexion
        int *new_client_socket = malloc(sizeof(int));
        if (new_client_socket == NULL)
        {
            perror("malloc");
            continue;
        }

        *new_client_socket = checked(accept(server_socket, (struct sockaddr *)&client_address, &client_len), "accept");
        if (*new_client_socket < 0)
        {
            perror("accept");
            free(new_client_socket);
            continue;
        }

        // Vérifier si le nombre de connexions atteint la limite
        if (count_active_clients() >= MAX_CLIENTS)
        {
            printf("Nombre maximal de clients atteint. Rejet de la connexion.\n");
            close(*new_client_socket);
            free(new_client_socket);
            continue;
        }

        printf("Nouvelle connexion acceptée.\n");

        // Créer un thread pour gérer le client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, client_thread, new_client_socket) != 0)
        {
            perror("pthread_create");
            close(*new_client_socket);
            free(new_client_socket);
        }
        pthread_detach(thread_id); // Détacher le thread pour éviter les fuites de ressources
    }
}
