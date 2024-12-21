#include "lecture.h"

void *readerThread(void *arg)
{
   pthread_mutex_t mutex;
   pthread_mutex_init(&mutex, NULL);
   signal(SIGPIPE, pipe_closure);
   Arguments *argv = (Arguments *)arg;
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

            printf("%s", buffer);
         }
         else
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
            msg = popStr(memory);
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
            code = addStr(memory, buffer);
         }
      }
      memset(buffer, 0, sizeof(buffer));
      pthread_mutex_unlock(&mutex);
   }
   msg = popStr(memory);
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
