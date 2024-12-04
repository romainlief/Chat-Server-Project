#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>


#define MAX_CLIENTS 1000
#define MAX_LEN_MESSAGE 1024
#define PORT_PAR_DEFAULT 1234

typedef struct {
    int socket_fd;                   // Descripteur du socket du client
    struct sockaddr_in address;     // Adresse du client
    pthread_t thread_id;            // Thread associé
    int is_active;                  // Indique si le client est actif
} client_t;

client_t clients[MAX_CLIENTS];     // Tableau des clients


int main(int argc, char* argv[]) {
   int port = (argc > 1) ? atoi(argv[1]) : PORT_PAR_DEFAULT;

   int server_socket;
   struct sockaddr_in server_addr;

   server_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (server_socket < 0) {
      perror("Erreur lors de la création du socket");
      exit(EXIT_FAILURE);
   }
   server_addr.sin_family = AF_INET; // Choix du protocole IPv4
   server_addr.sin_addr.s_addr = INADDR_ANY; // Accepter les connexions de n'importe quelle adresse
   server_addr.sin_port = htons(port); // Conversion du port en format réseau

   return 0;
}
