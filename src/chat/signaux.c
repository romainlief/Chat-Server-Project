#include "signaux.h"

void pipe_closure(int sig)
{
   fclose(stdin);
}

void ext(int sig)
{
   exit(4);
}
