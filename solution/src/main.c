#include <stdio.h>

#include "parametres.h"
#include "signal.h"
#include "mempartagee.h"
#include "processus.h"

int main(int argc, char* argv[]) {
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

   int retour = CreerProcessus(argv[1], argv[2], &options, messageSuspendu);

   LibererMessageSuspendu(messageSuspendu);

   return retour;
}
