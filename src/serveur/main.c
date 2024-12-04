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
#define MAX_CONNEXIONS 5


typedef struct {
    int socket_fd;                   // Descripteur du socket du client
    struct sockaddr_in address;     // Adresse du client
    pthread_t thread_id;            // Thread associé
    int is_active;                  // Indique si le client est actif
} client_t;


client_t clients[MAX_CLIENTS];     // Tableau des clients

/**
 * @brief Fonction de gestion des erreurs
 * 
 * @param ret la valeur de retour de la fonction à vérifier
 * @param calling_function le nom de la fonction appelante
 * @return ret la valeur de retour de la fonction à vérifier
 */
int checked(int ret, char* calling_function) {
   if (ret < 0) {
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
int get_server_port() {
   int port = PORT_PAR_DEFAULT; 
   char* port_env = getenv("PORT_SERVEUR"); // Récupération du port depuis la variable d'environnement

   if (port_env != NULL) {
      int env_port = atoi(port_env); 
      if (env_port > 0 && env_port <= 65535) {
         port = env_port;
      }
   }

   return port;
}

int init_server(int port) {
   int server_socket;
   struct sockaddr_in address;

   // Création du socket
   server_socket = checked(socket(AF_INET, SOCK_STREAM, 0), "socket");

   // Configuration de l'adresse du serveur
   address.sin_family = AF_INET; // Choix du protocole IPv4
   address.sin_addr.s_addr = INADDR_ANY; // Accepter les connexions de n'importe quelle adresse
   address.sin_port = htons(port); // Conversion du port en format réseau

   checked(bind(server_socket, (struct sockaddr*)&address, sizeof(address)), "bind"); // Liaison du socket à l'adresse et au port spécifiés
   checked(listen(server_socket, MAX_CONNEXIONS), "listen"); // Mise en écoute du serveur

   //printf("Serveur démarré sur le port %d\n", port);

   return server_socket;
}


int main(void) {
   int port = get_server_port();
   int server_socket = init_server(port);   

   return 0;
}
