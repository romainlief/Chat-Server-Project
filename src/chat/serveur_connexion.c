#include "serveur_connexion.h"


int checkIP()
{
    const char *var_name = "IP_SERVEUR";
    const char *delimiter = ".";
    char *ip = getenv(var_name);

    int n_number = 0;

    char *numbre_str = strtok(ip, delimiter);
    while (numbre_str != NULL)
    {

        if (0 <= atoi(numbre_str) && atoi(numbre_str) < 256)
        {
            n_number++;
        }

        numbre_str = strtok(NULL, delimiter);
    }

    return n_number == 4;
}

int checkPort()
{
    const char *var_name = "PORT_SERVEUR";
    char *port = getenv(var_name);

    return (BORNES_PORT_MIN <= atoi(port) && atoi(port) <= BORNES_PORT_MAX);
}

