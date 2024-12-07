#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# include <sys/types.h>
# include <sys/socket.h>
#include <arpa/inet.h>


#include "parametres.h"
#include "signal.h"
#include "mempartagee.h"
#include "processus.h"




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
      printf("ici");

   }
   else{
      inet_pton(AF_INET, ip, &serv_addr.sin_addr);
   }


   if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
      perror("SOCKET NON FAIT");
      exit(1);
   }


   LibererMessageSuspendu(messageSuspendu);

   return 0;
}

