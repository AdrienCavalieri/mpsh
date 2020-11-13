#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mkdir.h"

int mkdir_mpsh(int argc, char* args[]) {

    if( argc < 2 ) {
        fprintf(stderr, "mkdir: Missing argument.\n");
        return 1;
    }

    if( argc > 3 || (argc == 3 && args[1][0] != '-') ) {
        fprintf(stderr, "mkdir: %s.\n", strerror(E2BIG));
        return 1;
    }
    
    int i = 1;
    if(args[i][0] == '-') {
        if(args[i][1] != 'p' || strlen(args[i]) > 2 ) {
            fprintf(stderr, "mkdir: Invalid option: %s.\n", args[i]);
            return 1;
        }
        i++;
        if( i >= argc ) {
            fprintf(stderr, "mkdir: Missing argument.\n");
            return 1;
        }
    }

    if( mkdir( args[i], 0777 ) && i == 1)
        fprintf(stderr, "mkdir: Cannot create directory '%s': %s\n", args[i], strerror(errno));

    return 0;
}
