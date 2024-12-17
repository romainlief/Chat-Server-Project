#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# include <sys/types.h>
# include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#include "parametres.h"
// #include "signal.h"
// #include "mempartagee.h"
#include <signal.h>


#define BUFFER_SIZE 20

typedef struct {
   int* socket;
   OptionsProgramme options;
   char utilisateur[30]; 
} Arguments;

typedef struct {
  size_t taille;
  char* valeurs;
} liste_t;



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

liste_t create_mem() {
  liste_t ls = {0, NULL};
  ls.valeurs = (char *)malloc(BUFFER_SIZE);
  if (ls.valeurs == NULL) {
    perror("malloc()");
    free(ls.valeurs);
    exit(1);
  }
  return ls;
}


char* popStr(liste_t* ls) {
  if (ls->taille == 0) {
      printf("Memory is empty\n");
      return NULL;
  }

  size_t idx = 0;
  do {
    idx++;
  } while (ls->valeurs[idx] != '\0' && ls->taille >= idx);
  
  idx++; // to fit in '\0'
  ls->taille -= idx;

  char* retStr = (char *)malloc(idx + 1);
  if (retStr == NULL) {
    perror("malloc()");
    free(retStr);
    exit(1);
  }
  memcpy(retStr, ls->valeurs, idx); 
  memmove(ls->valeurs, ls->valeurs + idx, ls->taille); // shifting values to the left

  return retStr;

}

char* retStr(char* mem) {
    if (mem == NULL || mem[0] == '\0') {
        printf("Memory is empty\n");
        return NULL;
    }
    return mem;
}


int addStr(liste_t* ls, const char* str) {
    size_t str_len = strlen(str);

    if (ls->taille + str_len + 1 >= BUFFER_SIZE) {
        printf("Memory is full\n");
        return -1;
    }
    
    memcpy(ls->valeurs + ls->taille , str, str_len);
    ls->taille += str_len;
    ls->valeurs[ls->taille] = '\0';
    ls->taille++;
    return 0;
}

void * readerThread(void *arg){
   Arguments * argv = (Arguments *) arg;
   char* msg = NULL;
   char buffer[256];
   int * socket = argv->socket;
   liste_t memory;
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
         code = addStr(&memory, buffer);
         if(code == -1){
            msg = popStr(&memory);
            while(msg != NULL){
               printf("%s", msg);
               free(msg);
               msg = popStr(&memory);
            }
            code = addStr(&memory, buffer);
         }
         
      }

   }
   msg = popStr(&memory);
   while(msg != NULL){
      printf("%s", msg);
      free(msg);
      msg = popStr(&memory);
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




   OptionsProgramme options;
   GererParameteres(argc, argv, &options);


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
   

   while((code = getline(&message, &size_mess, stdin))>0){
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


   // LibererMessageSuspendu(messageSuspendu);
   close(sock);
   return 0;
}

