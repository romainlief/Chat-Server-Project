#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# include <sys/types.h>
# include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#include "parametres.h"
#include "signal.h"
#include "mempartagee.h"
#include "processus.h"

typedef struct {
   int* socket;
   OptionsProgramme options;
   char utilisateur[30]; 
} Arguments;



int checkIP(){
   const char * var_name = "IP_SERVEUR";
   const char* delimiter = ".";
   char * ip = getenv(var_name);

   int n_numbre = 0;

   char * numbre_str = strtok(ip, delimiter);
   while(numbre_str != NULL){

      if(0 <= atoi(numbre_str) && atoi(numbre_str) < 256 ){
         n_numbre ++;
      }

      numbre_str = strtok(NULL, delimiter);
   }


   return n_numbre == 4;
}

int checkPort(){
   const char * var_name = "PORT_SERVEUR";
   char * port = getenv(var_name);

   return( 0 < atoi(port) && atoi(port) <= 65535);
   }
   

void * readerThread(void *arg){
   Arguments * argv = (Arguments *) arg;
   char* message = NULL;
   char buffer[256];

   while(read(*argv->socket, buffer, sizeof(buffer)) > 0){
      char * name = strtok(buffer, " ");
      char * message = strtok(NULL, "");
      if (argv->options.modeBot) {
         printf("[%s] %s", name, buffer);
      } else {
         printf("[\x1B[4m%s\x1B[0m] %s", name, buffer);
      }


      fflush(stdout);
      

      return NULL;

   }
}

void* writerThread(void *arg){
   Arguments * argv = (Arguments *) arg;
   
   int *soket = argv->socket;
   char* message = NULL;
   size_t size_mess = 0;
   ssize_t code;

   pthread_t second_thread;
   pthread_create(&second_thread, NULL, &readerThread, &socket);
   pthread_join(second_thread, NULL);

   while((code = getline(&message, &size_mess, stdin)) > 0){
      // veroiller socket
      if (!argv->options.modeBot) {
         printf("[\x1B[4m%s\x1B[0m] %s", argv->utilisateur, message);
         fflush(stdout);
      }

      write(*soket, message, sizeof(message));
      // deverouiller socket
   }

   
   return NULL;
}




int main(int argc, char* argv[]) {

   const char * port_name = "PORT_SERVEUR";
   const char * port_value = "1234";
   setenv(port_name, port_value, 1);
   
   const char * IP_name = "IP_SERVEUR";
   const char * IP_value = "127.0.0.1";
   setenv(IP_name, IP_value, 1);



   MessageSuspendu* messageSuspendu;
   OptionsProgramme options;

   GererParameteres(argc, argv, &options);
   MiseEnPlaceGestionnairesSignaux();

   messageSuspendu = CreerMessageSuspendu();
   if (VerifierSigintEnAttente()) {
      return CODE_RETOUR_ARRET_SIGINT;
   } else if (messageSuspendu == NULL) {
      return CODE_RETOUR_ERREUR_AUTRE;
   }

   int port = 1234;  // valeur defaut
   char ip[] = "127.0.0.1";   // valeur defaut
   int sock = socket(AF_INET, SOCK_STREAM, 0);



   if(checkPort() == 1){
      char * port_value_str = getenv(port_name);

      port = atoi(port_value_str);
   }


   struct sockaddr_in serv_addr = {
   .sin_family = AF_INET ,
   .sin_port = htons(port)
   };


   if(checkIP() == 1){
      inet_pton(AF_INET, getenv(IP_name), &serv_addr.sin_addr);
   }
   else{
      inet_pton(AF_INET, ip, &serv_addr.sin_addr);
   }


   if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
      perror("SOCKET NON FAIT");
      exit(1);
   }
   

   Arguments argument;
   argument.socket = &sock;
   argument.options = options;
   strcpy(argument.utilisateur, argv[1]);


   // THREADS
   pthread_t origin_thread;
   
   pthread_create(&origin_thread, NULL, &writerThread, &argument);

   pthread_join(origin_thread, NULL);


   LibererMessageSuspendu(messageSuspendu);

   return 0;
}

