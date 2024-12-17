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
// #include "mempartagee.h"
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

char* create_mem() {
  char* mem = (char *)malloc(4096);
  if (mem == NULL) {
    perror("malloc()");
    free(mem);
    exit(1);
  }
  mem[0] = '\0';
  return mem;
}


char* retStr(char* mem) {
    if (mem == NULL || mem[0] == '\0') {
        printf("Memory is empty\n");
        return NULL;
    }
    return mem;
}

int addStr(char* mem, const char* str) {
    size_t current_len = strlen(mem);
    size_t str_len = strlen(str);

    if (current_len + str_len + 1 >= 4096) {
        printf("Memory is full\n");
        return -1;
    }
    strcat(mem, str);
    return 0;
}
   

void * readerThread(void *arg){
   Arguments * argv = (Arguments *) arg;
   char* message = NULL;
   char buffer[256];
   int * socket = argv->socket;
   char* memory;
   if(argv->options.affichageManuel){
      memory = create_mem();
   }
   printf("test\n");
   int code;
   while((read(*socket, buffer, sizeof(buffer))) > 0){
      
      if(!argv->options.affichageManuel){
         if (argv->options.modeBot) {
            
            printf("%s", buffer);
         } else {
            char * separators = "[]";
            char* tok = strtok(buffer, separators);
            printf("[\x1B[4m%s\x1B[0m]", tok);
            tok = strtok(NULL, "");
            printf("%s", tok);
         }

      }
      // fflush(stdout);
      
      else{
         code = addStr(memory, buffer);
         if(code == -1){
            retStr(memory);
            code = addStr(memory, buffer);
         }
         
      }

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
   send(sock, argument.utilisateur, strlen(argument.utilisateur), 0);

   char* message = NULL;
   size_t size_mess = 0;
   ssize_t code;

   pthread_t second_thread;
   pthread_create(&second_thread, NULL, &readerThread, &argument);
   char* token ;
   

   while((code = getline(&message, &size_mess, stdin)) > 0){
     char* verificateur = strdup(message);
      token = strtok(verificateur, " ");
      token = strtok(NULL, " ");
      if(token == NULL || strcmp(token, "\n") == 0 ){
         printf("nonvalide n\n");
         continue;
      }

      // veroiller socket
      if (!options.modeBot) {
         printf("[\x1B[4m%s\x1B[0m] %s", argument.utilisateur, message);
         fflush(stdout);
      }

      write(sock, message, sizeof(message));
      // deverouiller socket
   }

   pthread_join(second_thread, NULL);

   // THREADS
   // pthread_t origin_thread;
   
   // pthread_create(&origin_thread, NULL, &writerThread, &argument);

   // pthread_join(origin_thread, NULL);


   LibererMessageSuspendu(messageSuspendu);

   return 0;
}

