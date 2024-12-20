#include "serveur_manager.h"

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
client_t clients[MAX_CLIENTS];

int main(void)
{
    int server_socket = initServeur();

    run_server(server_socket);

    checked(close(server_socket), "close");

    return 0;
}
