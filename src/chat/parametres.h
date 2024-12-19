#ifndef PARAMETRES_H__
#define PARAMETRES_H__

#include <stdbool.h>

#define MAX_PSEUDO_DESTINATAIRE 30

typedef struct {
   bool modeBot;
   bool affichageManuel;
} OptionsProgramme;

/**
 * Récupère les options et vérifie leur conformité avec les consignes.
 *
 * @param argc Le nombre d'arguments du programme.
 * @param argv Une liste de chaînes de caractères avec les paramètres
 *    du programme.
 * @param options Contiendra les options détectées.
 **/
void GererParameteres(int argc, char* argv[], OptionsProgramme* options);

#endif
