#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "mempartagee.h"

void FlushBuffer(MessageSuspendu* messageSuspendu) {
   fwrite((char*)messageSuspendu->message, sizeof(char), messageSuspendu->indiceFinExclus, stdout);
   messageSuspendu->indiceFinExclus = 0;
}

void AjouterMessageBuffer(MessageSuspendu* messageSuspendu, const char* message) {
   size_t aEcrire = strlen(message);
   size_t tailleRestante;

   while (aEcrire > 0) {
      tailleRestante = TAILLE_MAXIMALE_AVANT_FLUSH - messageSuspendu->indiceFinExclus;
      if (aEcrire < tailleRestante) {
         strncpy((char*)&messageSuspendu->message[messageSuspendu->indiceFinExclus], message, aEcrire);
         messageSuspendu->indiceFinExclus += aEcrire;
         aEcrire = 0;
      } else {
         aEcrire -= tailleRestante;
         strncpy((char*)&messageSuspendu->message[messageSuspendu->indiceFinExclus], message, tailleRestante);
         FlushBuffer(messageSuspendu);
         message += tailleRestante;
         messageSuspendu->indiceFinExclus = 0;
      }
   }
}

MessageSuspendu* CreerMessageSuspendu(void) {
   MessageSuspendu* messageSuspendu = mmap(NULL, sizeof(MessageSuspendu), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
   if (messageSuspendu == NULL) {
      perror("mmap()");
      return NULL;
   }

   messageSuspendu->indiceFinExclus = 0u;

   return messageSuspendu;
}

void LibererMessageSuspendu(MessageSuspendu* messageSuspendu) {
   munmap(messageSuspendu, sizeof(MessageSuspendu));
}
