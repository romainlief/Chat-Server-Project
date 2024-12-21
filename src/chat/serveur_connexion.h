#ifndef SERVEUR_CONNEXION_H
#define SERVEUR_CONNEXION_H

#include "constants.h"

#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# include <sys/types.h>
# include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include "parametres.h"

#include "struct.h"

int checkIP();

int checkPort();



#endif // SERVEUR_CONNEXION_H
