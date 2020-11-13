#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


int umask_mpsh(int argc, char* args[]){

    if( argc == 1 ) {
        mode_t mask = umask( 0 );
        umask(mask);
        printf("%04o\n", mask);
        return 1;
    }
       
    if( argc > 2 ) {
        fprintf(stderr, "umask: %s.\n", strerror(E2BIG));
        return 1;
    }


    // On verifie si le c'est un nombre.
    char* endptr = NULL;
    strtol(args[1], &endptr, 10);
    if( *endptr != '\0' ) {       
        fprintf(stderr, "umask: %s.\n", strerror(EINVAL));
        return 1;
    }
  
    // On convertit la chaine de caracteres en long et
    // on verifie qu'il est en base 8.
    endptr = NULL;
    mode_t val = strtol(args[1], &endptr, 8);

    if( *endptr != '\0' ) {
       fprintf(stderr, "umask: %s: octal number out of range.\n", args[1]);
       return 1;
    }
    
    umask( val );
    
    return 0;  
}
