#include "parametres.h"
#include "serveur_connexion.h"
#include "memory.h"
#include "signaux.h"
#include "lecture.h"

// TODO: mettre le soulignement dans manuel mode

extern liste_t memoire;
extern OptionsProgramme options;

void process_message(char *message, size_t size_mess, int sock, OptionsProgramme options, Arguments argument);

int main(int argc, char *argv[])
{
   
   GererParameteres(argc, argv, &options);

   signal(SIGINT, ext); // exit 4 si SIGINT

   const char *port_name = "PORT_SERVEUR";
   const char *port_value = "1234";
   setenv(port_name, port_value, 1);

   const char *IP_name = "IP_SERVEUR";
   const char *IP_value = "127.0.0.1";
   setenv(IP_name, IP_value, 1);

   int port = PORT_PAR_DEFAULT; // valeur defaut
   char ip[] = "127.0.0.1";     // valeur defaut
   int sock = socket(AF_INET, SOCK_STREAM, 0);

   if (checkPort() == 1)
   {
      char *port_value_str = getenv(port_name);
      port = atoi(port_value_str);
   }

   struct sockaddr_in serv_addr = {
       .sin_family = AF_INET,
       .sin_port = htons(port)};

   if (checkIP() == 1)
   {
      inet_pton(AF_INET, getenv(IP_name), &serv_addr.sin_addr);
   }
   else
   {
      inet_pton(AF_INET, ip, &serv_addr.sin_addr);
   }

   if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
   {
      perror("SOCKET NON FAIT");
      exit(1);
   }

   if (!options.affichageManuel)  // Igniorer SIGINT si pas de Manuel Mod
   { // igniore signint
      sigset_t set;
      sigemptyset(&set);
      sigaddset(&set, SIGINT);
      pthread_sigmask(SIG_BLOCK, &set, NULL);
   }
   else  // Si Manuel mod, SIGINT vide la mémoire
   {
      signal(SIGINT, set_vider);
   }

   Arguments argument;
   argument.socket = &sock;
   argument.options = options;
   argument.memoir = &memoire;
   strcpy(argument.utilisateur, argv[1]);
   send(sock, argument.utilisateur, strlen(argument.utilisateur), 0); // envoie du nom d'utilisateur sur le socket

   char *message = NULL;
   size_t size_mess = 0;
   ssize_t code;

   pthread_t second_thread;
   pthread_create(&second_thread, NULL, &readerThread, &argument); //creation thread lecture

   while ((code = getline(&message, &size_mess, stdin))) {
      if (code == -1)
      {
         break; // tant que stdin est ouvert on peut écrire
      }
      process_message(message, size_mess, sock, options, argument);
   }

   free(message);
   shutdown(sock, SHUT_RD); // fermeture de lecture du socket permettant au thread lecture de se terminer
   close(sock);
   pthread_join(second_thread, NULL);
   return 0;
}


void process_message(char *message, size_t size_mess, int sock, OptionsProgramme options, Arguments argument) {
    int spacecounter = 0;
    int idx = 0;

    while (message[idx] == ' ') {
        spacecounter++;
        idx++;
    }

    char *verificateur = strdup(message);  // verification de message valide: 2 mots min
    char *token = strtok(verificateur, " ");
    token = strtok(NULL, " ");
    if (token == NULL || strcmp(token, "\n") == 0 || spacecounter > 0) {
        free(verificateur);
        printf("Message non valide\n");
        return;
    }

    char temp[size_mess];
    memcpy(temp, message, size_mess);

    if (!options.modeBot) {
        printf("[\x1B[4m%s\x1B[0m] %s", argument.utilisateur, temp);
        fflush(stdout);
    }

    if (options.affichageManuel) {
        char *msg = popStr(&memoire);
        while (msg != NULL) { // vidage mémoire 
            if (options.modeBot) { // ecriture message sans soulignement
                printf("%s", msg);
            } else {    // ecriture message avec soulignement
                char *separators = "[]";
                char *tok = strtok(msg, separators);
                printf("[\x1B[4m%s\x1B[0m]", tok);
                tok = strtok(NULL, "");
                printf("%s", tok);
            }
            free(msg);
            msg = popStr(&memoire);
        }
    }

    write(sock, temp, sizeof(temp));
    free(verificateur);
}
