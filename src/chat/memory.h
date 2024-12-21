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
 * @brief Permet d'extraire le premier string entré dans la mémoire
 * 
 * @param ls Mémoire
 * @return chaine de caractère entrée en premier
 */
char *popStr(liste_t *ls);

// /**
//  * @brief Permet écritur en première case mémoire. 
//  * 
//  * @param mem pot
//  */
// char *retStr(char *mem);

/**
 * @brief Ajoute un string a la mémoire
 * 
 * @param ls pointeurvers mémoire
 * @param str poitneur vers chaine de caractères
 * @return 0 si la chaine a été ajoutée, -1 si non. 
 */
int addStr(liste_t *ls, const char *str);

/**
 * @brief Gestionnair de signial sigint: vide la mémoire en imprimmant son contenu
 */
void set_vider(int sig);

#endif // MEMORY_H
