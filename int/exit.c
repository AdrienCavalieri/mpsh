#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "exit.h"
#include "history.h"
#include "../variable.h"

int exit_mpsh(int nb, char* args[])
{
    int val = nb > 1 ? atoi(args[1]) : atoi(get_var_value("?"));

    if (nb > 2)
        fprintf(stderr, "exit: %s.\n", strerror(E2BIG));

    export_history();
    
    exit(val);

    // Inutile mais pour eviter d'eventuels problemes de compilation.
    return 0;
}
