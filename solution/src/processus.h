#ifndef PROCESSUS_H__
#define PROCESSUS_H__

#include "parametres.h"
#include "mempartagee.h"

enum {
   CODE_RETOUR_NORMAL,
   CODE_RETOUR_PARAMETRES_MANQUANTS,
   CODE_RETOUR_PSEUDO_TROP_LONG,
   CODE_RETOUR_PSEUDO_CARACTERES_INVALIDES,
   CODE_RETOUR_ARRET_SIGINT,
   CODE_RETOUR_ERREUR_AUTRE
};


/**
 * Crée le processeus secondaire et démarre leur fonctionnement.
 *
 * @param pseudo Le pseudonyme de l'utilisatrice ou utilisateur.
 * @param destinataire Le pseudonyme du destinataire.
 * @param options Options utilisées dans le programme.
 * @param messageSuspendu Permet de mettre les messages reçus
 *    en attente lorsque le mode --manuel est activé. Est supposé
 *    déjà initialisé.
 *
 * @return Code de retour du chat.
 **/
int CreerProcessus(const char pseudo[], const char destinataire[], const OptionsProgramme* options, MessageSuspendu* messageSuspendu);

#endif
