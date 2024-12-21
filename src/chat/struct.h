#ifndef STRUCT_H
#define STRUCT_H

#include "parametres.h"
/**
 * @brief Structure de la mémoire
 */
typedef struct {
  size_t taille;
  char* valeurs;
} liste_t;

/**
 * @brief structure permettant de transmettre parametres essentiels entre threads
 */
typedef struct {
   int* socket;
   OptionsProgramme options;
   char utilisateur[MAX_LEN_PSEUDO]; 
   liste_t* memoir;
} Arguments;

#endif // STRUCT_H
