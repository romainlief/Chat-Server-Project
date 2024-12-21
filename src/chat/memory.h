#ifndef MEMORY_H
#define MEMORY_H

#include "constants.h"
#include "struct.h"

/**
 * @brief Initialisation de la mémoire
 * 
 * @return Mémoire initialisée
 */
liste_t create_mem();

/**
 * @brief
 */
char *popStr(liste_t *ls);

char *retStr(char *mem);

int addStr(liste_t *ls, const char *str);

void set_vider(int sig);

#endif // MEMORY_H
