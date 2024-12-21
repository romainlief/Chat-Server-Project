#ifndef MEMORY_H
#define MEMORY_H

#include "constants.h"
#include "struct.h"


liste_t create_mem();

char *popStr(liste_t *ls);

char *retStr(char *mem);

int addStr(liste_t *ls, const char *str);

void set_vider(int sig);

#endif // MEMORY_H
