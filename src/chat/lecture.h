#ifndef LECTURE_H
#define LECTURE_H

#include <pthread.h>

#include "parametres.h"
#include "constants.h"
#include "serveur_connexion.h"
#include "struct.h"
#include "memory.h"
#include "signaux.h"
/**
 * @brief Thread permettant lecture des message
 * 
 * @param arg parametre parmettant de passer les options du programme, le socket, ...
 */
void *readerThread(void *arg);

#endif // LECTURE_H
