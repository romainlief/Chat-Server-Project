#include "lecture.h"

void *readerThread(void *arg)
{
   pthread_mutex_t mutex;
   pthread_mutex_init(&mutex, NULL);
   signal(SIGPIPE, pipe_closure);
   Arguments *argv = (Arguments *)arg;  // Récupération des information de base données dans la main
   char *msg = NULL;
   char buffer[MAX_LEN_MESSAGE];
   int *socket = argv->socket;
   liste_t *memory = argv->memoir;
   if (argv->options.affichageManuel)
   {
      *memory = create_mem();
   }
   int code;
   while ((read(*socket, buffer, sizeof(buffer))) > 0)
   {
      pthread_mutex_lock(&mutex);
      if (!argv->options.affichageManuel)
      {
         if (argv->options.modeBot)
         { 

            printf("%s", buffer);  // écriture non soulignée si botmode
         }
         else  // écriture soulignée si pas de botmode
         {
            char *separators = "[]";
            char *tok = strtok(buffer, separators);
            printf("[\x1B[4m%s\x1B[0m]", tok);
            tok = strtok(NULL, "");
            printf("%s", tok);
         }
      }

      else
      {
         code = addStr(memory, buffer);
         printf("\a");
         fflush(stdout);
         if (code == -1)
         {
            msg = popStr(memory);  // Vidage de la mémoire
            while (msg != NULL)
            {
               if (argv->options.modeBot){ // écriture non soulignée si botmode
                  printf("%s", msg);
               } 
               else  // écriture soulignée si pas de botmode
               {
                  char *separators = "[]";
                  char *tok = strtok(msg, separators);
                  printf("[\x1B[4m%s\x1B[0m]", tok);
                  tok = strtok(NULL, "");
                  printf("%s", tok);
               }
               free(msg);
               msg = popStr(memory);
            }
            code = addStr(memory, buffer);
         }
      }
      memset(buffer, 0, sizeof(buffer)); // vidage du buffer
      pthread_mutex_unlock(&mutex);
   }
   msg = popStr(memory);  // si il reste des messages dans la mémoire on les évacue
   while (msg != NULL)
   {
      if (argv->options.modeBot){
         printf("%s", msg);
      }
      else
      {
         char *separators = "[]";
         char *tok = strtok(msg, separators);
         printf("[\x1B[4m%s\x1B[0m]", tok);
         tok = strtok(NULL, "");
         printf("%s", tok);
      }
      free(msg);
      msg = popStr(memory);
   }

   kill(getpid(), SIGPIPE);
   return NULL;
}
