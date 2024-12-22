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

        if (0 <= atoi(numbre_str) && atoi(numbre_str) < 256)  // verifie nombre valide
        {
            n_number++;
        }

        numbre_str = strtok(NULL, delimiter);
    }
    return n_number == 4; // si 4 nombre valides, return 1
}

int checkPort()
{
    const char *var_name = "PORT_SERVEUR";
    char *port = getenv(var_name);

    return (BORNES_PORT_MIN <= atoi(port) && atoi(port) <= BORNES_PORT_MAX);
}

void init_chat(int *sock, struct sockaddr_in *serv_addr, const char *port_name, const char *IP_name)
{
    int port = PORT_PAR_DEFAULT; // valeur defaut
    char ip[] = "127.0.0.1";     // valeur defaut
    *sock = socket(AF_INET, SOCK_STREAM, 0);

    if (checkPort() == 1)
    {
        char *port_value_str = getenv(port_name);
        port = atoi(port_value_str);
    }

    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(port);

    if (checkIP() == 1)
    {
        inet_pton(AF_INET, getenv(IP_name), &serv_addr->sin_addr);
    }
    else
    {
        inet_pton(AF_INET, ip, &serv_addr->sin_addr);
    }
}
