#include "process.h"

extern liste_t memoire;
extern OptionsProgramme options;

void process_message(char *message, size_t size_mess, int sock, OptionsProgramme options, Arguments argument)
{
   int spacecounter = 0;
   int idx = 0;

   while (message[idx] == ' ')
   {
      spacecounter++;
      idx++;
   }

   char *verificateur = strdup(message); // verification de message valide: 2 mots min
   char *token = strtok(verificateur, " ");
   token = strtok(NULL, " ");
   if (token == NULL || strcmp(token, "\n") == 0 || spacecounter > 0)
   {
      free(verificateur);
      printf("Message non valide\n");
      return;
   }

   char temp[size_mess];
   memcpy(temp, message, size_mess);

   if (!options.modeBot)
   {
      printf("[\x1B[4m%s\x1B[0m] %s", argument.utilisateur, temp);
      fflush(stdout);
   }

   if (options.affichageManuel)
   {
      char *msg = popStr(&memoire);
      while (msg != NULL)
      { // vidage mémoire
         if (options.modeBot)
         { // ecriture message sans soulignement
            printf("%s", msg);
         }
         else
         { // ecriture message avec soulignement
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
