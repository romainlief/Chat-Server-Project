#include <stdatomic.h>
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
  size_t taille;
  char* valeurs;
} liste_t;

typedef struct {
   int* socket;
   OptionsProgramme options;
   char utilisateur[30]; 
   liste_t* memoir;
} Arguments;



liste_t memoire;



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
   char buffer[1024];
   int * socket = argv->socket;
   liste_t* memory = argv->memoir;
   if(argv->options.affichageManuel){
      *memory = create_mem();
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
      
      
      else{
         code = addStr(memory, buffer);
         printf("\a");
         fflush(stdout);
         if(code == -1){
            msg = popStr(memory);
            while(msg != NULL){
               printf("%s", msg);
               free(msg);
               msg = popStr(memory);
            }
            code = addStr(memory, buffer);
         }
         
      }

   }
   msg = popStr(memory);
   while(msg != NULL){
      printf("%s", msg);
      free(msg);
      msg = popStr(memory);
   }
   fclose(stdin);
   return NULL;
}



void ext(int sig){
   exit(4);
}

void set_vider(int sig){
   char* msg = popStr(&memoire);
   while(msg != NULL){
      printf("%s", msg);
      free(msg);
      msg = popStr(&memoire);
   }
}





int main(int argc, char* argv[]) {
   OptionsProgramme options;
   GererParameteres(argc, argv, &options);

   
   signal(SIGINT, ext); // exit 4 si SIGINT
   

   const char * port_name = "PORT_SERVEUR";
   const char * port_value = "1234";
   setenv(port_name, port_value, 1);
   
   const char * IP_name = "IP_SERVEUR";
   const char * IP_value = "127.0.0.1";
   setenv(IP_name, IP_value, 1);





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
   
   // signal(SIGINT, emptyMemory);
   
   if(!options.affichageManuel){   //igniore signint
      sigset_t set;
      sigemptyset(&set);
      sigaddset(&set, SIGINT);
      pthread_sigmask(SIG_BLOCK, &set, NULL);
   }
   else{
      signal(SIGINT, set_vider);
   }
   

   Arguments argument;
   // liste_t memoire;
   argument.socket = &sock;
   argument.options = options;
   argument.memoir = &memoire;
   strcpy(argument.utilisateur, argv[1]);
   send(sock, argument.utilisateur, strlen(argument.utilisateur), 0);

   char* message = NULL;
   size_t size_mess = 0;
   ssize_t code;

   pthread_t second_thread;
   pthread_create(&second_thread, NULL, &readerThread, &argument);
   
   char* token ;
   
   printf("ouverture\n");
   
   while((code = getline(&message, &size_mess, stdin))){
      char* verificateur = strdup(message);
      token = strtok(verificateur, " ");
      token = strtok(NULL, " ");
      if(token == NULL || strcmp(token, "\n") == 0 ){
         if(code == -1){
            break;
         }
         printf("nonvalide\n");
         continue;
      }

      char temp[size_mess];
      memcpy(temp, message, size_mess);

      

      if (!options.modeBot) {
         printf("[\x1B[4m%s\x1B[0m] %s", argument.utilisateur, temp);
         fflush(stdout);
      }

      if(options.affichageManuel){
         char* msg = popStr(&memoire);
         while(msg != NULL){
            printf("%s", msg);
            free(msg);
            msg = popStr(&memoire);
         }
      }
      
      write(sock, temp, sizeof(temp));
   }
   
   
     
   // LibererMessageSuspendu(messageSuspendu);
   close(sock);
   
   pthread_join(second_thread, NULL);  
   return 0;
}

