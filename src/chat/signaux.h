#ifndef SIGNAUX_H
#define SIGNAUX_H

#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

/**
 * @brief Fermeture du stdin en cas de SIGPIPE
 */
void pipe_closure(int sig);

/**
 * @brief exit(4) en cas de SIGINT prématuré
 */
void ext(int sig);

#endif // SIGNAUX_H
