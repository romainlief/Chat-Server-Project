#ifndef PROCESS_H
#define PROCESS_H

#include "struct.h"
#include "memory.h"
#include "parametres.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief Traitement du message entrant
 * 
 * @param message message entrant
 * @param size_mess taille du message
 * @param sock socket
 * @param options options du programme
 * @param argument arguments
 */
void process_message(char *message, size_t size_mess, int sock, OptionsProgramme options, Arguments argument);

/**
 * @brief Vérifie si le message est valide
 * 
 * @param message message
 * @param size_mess taille du message
 * @return int 1 si valide, 0 si non
 */
int is_valid_message(char *message, size_t size_mess);

#endif // PROCESS_H
