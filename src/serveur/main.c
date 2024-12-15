#include "serveur_manager.h"

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
client_t clients[MAX_CLIENTS];

int main(void)
{
    int port = getServeurPort();
    int server_socket = initServeur(port);

    run_server(server_socket);

    close(server_socket);

    return 0;
}
