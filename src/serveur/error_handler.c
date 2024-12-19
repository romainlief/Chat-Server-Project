#include "error_handler.h"

int checked(int ret, char *calling_function)
{
    if (ret < 0)
    {
        perror(calling_function);
        exit(EXIT_FAILURE);
    }
    return ret;
}
