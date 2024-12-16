#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdlib.h>
#include <stdio.h>


/**
 * @brief Fonction de gestion des erreurs
 *
 * @param ret la valeur de retour de la fonction à vérifier
 * @param calling_function le nom de la fonction appelante
 * @return ret la valeur de retour de la fonction à vérifier
 */
int checked(int ret, char *calling_function);


#endif // ERROR_HANDLER_H
