#ifndef SIGNAUX_H
#define SIGNAUX_H

#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

void pipe_closure(int sig);

void ext(int sig);

#endif // SIGNAUX_H
