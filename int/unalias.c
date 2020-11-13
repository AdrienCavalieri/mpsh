#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "alias.h"
#include "unalias.h"


int unalias(int argc, char* args[]){

    if( argc == 1 ) {
        fprintf(stderr, "unalias: Missing argument.\n");
        return 1;
    }

    if( argc > 2 ) {
        fprintf(stderr, "unalias: %s\n", strerror(E2BIG));
        return 1;
    }
    
    if( !remove_alias(args[1]) ) {
        fprintf(stderr, "unalias: alias '%s' doesn't exists.\n", args[1]);
        return 1;
    }
    
    return 0;
}
