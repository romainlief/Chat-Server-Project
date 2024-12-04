#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "processus.h"
#include "parametres.h"
#include "signal.h"
#include "io.h"

static void GenererNomPipe(char chemin[], const char pseudo[], const char destinataire[]) {
   sprintf(chemin, "/tmp/%s-%s.chat", pseudo, destinataire);
}

static int GenererDescripteurLecture(const char pseudo[], const char destinataire[]) {
   char chemin[1024];
   int fd;

   GenererNomPipe(chemin, pseudo, destinataire);
   if (mkfifo(chemin, 0666) < 0 && errno != EEXIST) {
      perror("GenererDescripteurLecture::mkfifo()");
      return -1;
   }

   fd = open(chemin , O_RDONLY);
   if (fd == -1) {
      perror(chemin);
   }

   return fd;
}

static int GenererDescripteurEcriture(const char pseudo[], const char destinataire[]) {
   char chemin[1024];
   int fd;

   GenererNomPipe(chemin, destinataire, pseudo);
   sprintf(chemin, "/tmp/%s-%s.chat", destinataire, pseudo);
   if (mkfifo(chemin, 0666) < 0 && errno != EEXIST) {
      perror("GenererDescripteurEcriture::mkfifo()");
      return -1;
   }

   fd = open(chemin , O_WRONLY);
   if (fd == -1) {
      unlink(chemin);
      perror(chemin);
   }

   return fd;
}

static bool GererEchecFgets(MessageSuspendu* messageSuspendu, bool affichageManuel) {
   if (feof(stdin)) {
      return false;
   } else {
      if (VerifierSigintEnAttente()) {
         if (affichageManuel) {
            TraiterSigint(messageSuspendu);
            clearerr(stdin);
            return true;
         }
         return false;
      } else if (VerifierSigusr1EnAttente()) {
         return false;
      } else {
         perror("fgets()");
         return false;
      }
   }
}

static int EnvoyerMessagesStdin(const char pseudo[], const char destinataire[], const OptionsProgramme* options, MessageSuspendu* messageSuspendu) {
   char buffer[1024];
   int fdEcriture = GenererDescripteurEcriture(pseudo, destinataire);
   bool messageEnMorceaux = false;

   if (fdEcriture == -1) {
      if (VerifierSigintEnAttente()) {
         return CODE_RETOUR_ARRET_SIGINT;
      }
      return CODE_RETOUR_ERREUR_AUTRE;
   }

   while (1) {
      if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
         // fgets() peut échouer à cause d'une erreur (p.ex., la réception d'un
         // signal) ou d'une fin de flux. Les erreurs seront critiques sauf
         // pour SIGINT en mode manuel.
         if (!GererEchecFgets(messageSuspendu, options->affichageManuel))
            break;
         continue;
      }

      if (!options->modeBot) {
         if (!messageEnMorceaux)
            printf("[\x1B[38;2;0;204;255m\x1B[4m%s\x1B[0m] ", pseudo);
         printf("%s", buffer);
      }
      
      uint32_t longueur = (uint32_t)strnlen(buffer, sizeof(buffer));
      messageEnMorceaux = (longueur > 0 && buffer[longueur - 1] != '\n');

      if (!messageEnMorceaux && options->affichageManuel) {
         FlushBuffer(messageSuspendu);
      }

      if (!WriteStreamMessage(fdEcriture, buffer, longueur, messageSuspendu)) {
         break;
      }

      if (VerifierSigintEnAttente()) {
         TraiterSigint(messageSuspendu);
      } else if (VerifierSigusr1EnAttente()) {
         break;
      }
   }

   close(fdEcriture);
   return CODE_RETOUR_NORMAL;
}

static void AfficherPseudos(char messageAffiche[], const char destinataire[], const OptionsProgramme* options, MessageSuspendu* messageSuspendu) {
   if (!options->modeBot) {
      sprintf(messageAffiche, "[\x1B[38;2;187;119;255m\x1B[4m%s\x1B[0m] ", destinataire);
   } else {
      sprintf(messageAffiche, "[%s] ", destinataire);
   }

   if (options->affichageManuel) {
      putchar('\a');
      AjouterMessageBuffer(messageSuspendu, messageAffiche);
   } else {
      printf("%s", messageAffiche);
   }
}

static int RecevoirMessages(const char pseudo[], const char destinataire[], const OptionsProgramme* options, MessageSuspendu* messageSuspendu) {
   uint32_t taille;
   char buffer[1025], messageAffiche[1025];
   int fdLecture = GenererDescripteurLecture(pseudo, destinataire);
   char copiePseudo[MAX_PSEUDO_DESTINATAIRE + 1];

   // Les messages sont lus sur stdin par paquets de 1024 octets.
   // Il n'y a cependant pas de limite imposée sur la longueur d'un
   // message. Les messages plus longs sont donc affichés par
   // paquets de 1024 octets mais il ne faut afficher le pseudo de
   // l'émetteur qu'une seule fois. Lorsque messageEnMorceaux est
   // `true`, cela signifie que le message lu sur le pipe est un
   // des blocs de 1024 octets d'un long message (mais pas le
   // premier bloc).
   bool messageEnMorceaux = false;

   if (fdLecture == -1) {
      if (VerifierSigintEnAttente()) {
         return CODE_RETOUR_ARRET_SIGINT;
      }
      return CODE_RETOUR_ERREUR_AUTRE;
   }

   strncpy(copiePseudo, (char*)destinataire, MAX_PSEUDO_DESTINATAIRE);

   buffer[sizeof(buffer) - 1] = '\0';
   while (ReadStreamMessage(fdLecture, buffer, sizeof(buffer) - 1, &taille, messageSuspendu)) {
      if (VerifierSigusr1EnAttente()) {
         break;
      }

      buffer[taille] = '\0';

      if (!messageEnMorceaux) {
         AfficherPseudos(messageAffiche, destinataire, options, messageSuspendu);
      }

      sprintf(messageAffiche, "%s", buffer);
      if (options->affichageManuel) {
         AjouterMessageBuffer(messageSuspendu, messageAffiche);
      } else {
         printf("%s", messageAffiche);
         fflush(stdout);
      }

      // Le message suivant est un morceau si celui-ci ne s'est pas
      // terminé par un '\n'.
      messageEnMorceaux = (buffer[taille - 1] != '\n');

      if (VerifierSigusr1EnAttente()) {
         break;
      }
   }

   close(fdLecture);
   return CODE_RETOUR_NORMAL;
}

static void AttendreFinProcessus(pid_t pid) {
   int status, retour;
   do {
      retour = waitpid(pid, &status, 0);
   } while (retour == -1 || (!WIFEXITED(status) && !WIFSIGNALED(status)));
}

static void NettoyerPipes(const char pseudo[], const char destinataire[]) {
   char chemin[1024];

   GenererNomPipe(chemin, pseudo, destinataire);
   unlink(chemin);
   GenererNomPipe(chemin, destinataire, pseudo);
   unlink(chemin);
}

int CreerProcessus(const char pseudo[], const char destinataire[], const OptionsProgramme* options, MessageSuspendu* messageSuspendu) {
   pid_t pidLecteur = getpid();
   pid_t pidEcrivain = fork();
   int codeRetour = CODE_RETOUR_NORMAL;

   if (pidEcrivain == 0) {
      signal(SIGINT, SIG_IGN);
      codeRetour = RecevoirMessages(pseudo, destinataire, options, messageSuspendu);

      kill(pidLecteur, SIGUSR1);
      FlushBuffer(messageSuspendu);
   } else if (pidLecteur > 0) {
      codeRetour = EnvoyerMessagesStdin(pseudo, destinataire, options, messageSuspendu);

      kill(pidEcrivain, SIGUSR1);
      AttendreFinProcessus(pidEcrivain);
   } else {
      perror("fork()");
      codeRetour = CODE_RETOUR_ERREUR_AUTRE;
   }

   // Ce code est exécuté par le processus d'origine et le second
   // processus. Ceci nous garantit que, si au moins un des deux se
   // termine proprement (idem pour l'autre chat s'il y avait une
   // communication), alors les pipes nommés seront nettoyés.
   NettoyerPipes(pseudo, destinataire);

   return codeRetour;
}
