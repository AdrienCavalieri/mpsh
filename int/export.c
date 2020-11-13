#define  _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "../variable.h"
#include "../cmdmanager.h"
#include "export.h"


int export(int nb, char* args[]){


    if(nb < 2) {
        fprintf(stderr, "export: Missing argument.\n");
        return 1;
    }
        
    variable* v=malloc(sizeof(variable)); 

    for(int i=1; i<nb; i++){

        
        // s'il n'y a pas de '=';
        if( strchr(args[i], '=') == NULL ) {
            
            char* val = get_var_value(args[i]);
            if( val == NULL )
                val = "";
            
            // setenv fait des copies.
            setenv(args[i], val, 1);
            remove_var(args[i]);
            
        }else {
            
            if( !extract(args[i],v) )
                fprintf(stderr, "export: '%s': not a valid identifier.\n", args[i]);
            else {
                setenv(v->name, v->value, 1);             
                free(v->name);
                free(v->value);
            }
            
        }
        
    }

    free(v);

    return 0;
}
