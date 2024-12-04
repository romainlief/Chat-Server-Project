#ifndef MEM_PARTAGEE_H__
#define MEM_PARTAGEE_H__

#include <stddef.h>

#define TAILLE_MAXIMALE_AVANT_FLUSH ((size_t)4096)

typedef struct {
   volatile char message[TAILLE_MAXIMALE_AVANT_FLUSH];
   volatile size_t indiceFinExclus;
} MessageSuspendu;

/**
 * Retire et affiche les messages en attente sur stdout.
 *
 * @param buffer Messages en attente.
 **/
void FlushBuffer(MessageSuspendu* messageSuspendu);

/**
 * Ajoute un message à ceux en attente.
 *
 * @param buffer Messages en attente.
 * @param message Message à ajouter.
 **/
void AjouterMessageBuffer(MessageSuspendu* messageSuspendu, const char* message);

/**
 * Crée un système de messages en attente conservé en
 * mémoire partagée. Celui-ci doit être libéré à l'aide
 * de LibererMessageSuspend().
 *
 * @return Un système de messages en attente.
 **/
MessageSuspendu* CreerMessageSuspendu(void);

/**
 * Libère la mémoire utilisée pour les messages en attente.
 *
 * @param messageSuspendu Les messages en attente à libérer.
 **/
void LibererMessageSuspendu(MessageSuspendu* messageSuspendu);

#endif
