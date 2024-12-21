#ifndef STRUCT_H
#define STRUCT_H

#include "parametres.h"

typedef struct {
  size_t taille;
  char* valeurs;
} liste_t;

typedef struct {
   int* socket;
   OptionsProgramme options;
   char utilisateur[MAX_LEN_PSEUDO]; 
   liste_t* memoir;
} Arguments;

#endif // STRUCT_H
