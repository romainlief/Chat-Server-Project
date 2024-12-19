#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "parametres.h"

enum {
   CODE_RETOUR_NORMAL,
   CODE_RETOUR_PARAMETRES_MANQUANTS,
   CODE_RETOUR_PSEUDO_TROP_LONG,
   CODE_RETOUR_PSEUDO_CARACTERES_INVALIDES,
   CODE_RETOUR_ARRET_SIGINT,
   CODE_RETOUR_ERREUR_AUTRE
};

static void VerifierValiditePseudonymes(int argc, char* argv[]) {
   if (argc < 2) {
      fputs("chat pseudo_utilisateur [--bot] [--manuel]\n", stderr);
      exit(CODE_RETOUR_PARAMETRES_MANQUANTS);
   }

   if (strlen(argv[1]) > MAX_PSEUDO_DESTINATAIRE) {
      fprintf(stderr, "La longueur du pseudonyme ne peut excéder %d caractères.\n", MAX_PSEUDO_DESTINATAIRE);
      exit(CODE_RETOUR_PSEUDO_TROP_LONG);
   }

   const char caracteresInterdits[] = { '/', '-', '[', ']' };

   // for (int i = 1; i < 3; ++i) {
      for (unsigned int j = 0; j < sizeof(caracteresInterdits) / sizeof(*caracteresInterdits); ++j) {
         if (strchr(argv[1], caracteresInterdits[j]) != NULL) {
            fprintf(stderr, "Le caractère '%c' n'est pas autorisé dans un pseudonyme.\n", caracteresInterdits[j]);
            exit(CODE_RETOUR_PSEUDO_CARACTERES_INVALIDES);
         }
      }

      if (strcmp(argv[1], ".") == 0 || strcmp(argv[1], "..") == 0) {
         fprintf(stderr, "Le pseudonyme '%s' n'est pas autorisé.\n", argv[1]);
         exit(CODE_RETOUR_PSEUDO_CARACTERES_INVALIDES);
      }
   }
// }

static void ExtraireOptionsAdditionnelles(int argc, char* argv[], OptionsProgramme* options) {
   options->modeBot = false;
   options->affichageManuel = false;

   for (int i = 2; i < argc; ++i) {
      if (strcmp(argv[i], "--bot") == 0) {
         options->modeBot = true;
      } else if (strcmp(argv[i], "--manuel") == 0) {
         options->affichageManuel = true;
      }
   }
}

void GererParameteres(int argc, char* argv[], OptionsProgramme* options) {
   VerifierValiditePseudonymes(argc, argv);
   ExtraireOptionsAdditionnelles(argc, argv, options);
}
