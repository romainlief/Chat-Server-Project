#ifndef LECTURE_H
#define LECTURE_H

#include <pthread.h>

#include "parametres.h"
#include "constants.h"
#include "serveur_connexion.h"
#include "struct.h"
#include "memory.h"
#include "signaux.h"

void *readerThread(void *arg);

#endif // LECTURE_H
