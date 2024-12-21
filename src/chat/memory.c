#include "memory.h"

liste_t memoire;

liste_t create_mem()
{
    liste_t ls = {0, NULL};
    ls.valeurs = (char *)malloc(BUFFER_SIZE);
    if (ls.valeurs == NULL)
    {
        perror("malloc()");
        free(ls.valeurs);
        exit(1);
    }
    return ls;
}

char *popStr(liste_t *ls)
{
   if (ls->taille == 0)
   {
      return NULL;
   }

   size_t idx = 0;
   do
   {
      idx++;
   } while (ls->valeurs[idx] != '\0' && ls->taille >= idx);

   idx++; // to fit in '\0'
   ls->taille -= idx;

   char *retStr = (char *)malloc(idx + 1);
   if (retStr == NULL)
   {
      perror("malloc()");
      free(retStr);
      exit(1);
   }
   memcpy(retStr, ls->valeurs, idx);
   memmove(ls->valeurs, ls->valeurs + idx, ls->taille); // shifting values to the left

   return retStr;
}

char *retStr(char *mem)
{
   if (mem == NULL || mem[0] == '\0')
   {
      return NULL;
   }
   return mem;
}

int addStr(liste_t *ls, const char *str)
{
   size_t str_len = strlen(str);

   if (ls->taille + str_len + 1 >= BUFFER_SIZE)
   {
      return -1;
   }

   memcpy(ls->valeurs + ls->taille, str, str_len);
   ls->taille += str_len;
   ls->valeurs[ls->taille] = '\0';
   ls->taille++;
   return 0;
}

void set_vider(int sig)
{
   char *msg = popStr(&memoire);
   while (msg != NULL)
   {
      printf("%s", msg);
      free(msg);
      msg = popStr(&memoire);
   }
}
