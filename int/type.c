#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "../variable.h"
#include "../cmdmanager.h"
#include "alias.h"
#include "type.h"


static short get_type(char* name){
    char* c=get_alias_value(name);
    if(c!=NULL){
        printf("%s est un alias de %s\n",name,c);
    }else{
        int r=is_cmd(name);
        if(r==1){
            printf("%s est une commande interne\n",name);
        }else if (r==2){
            printf("%s est une commande externe\n", name);
        }else{
            printf("type: %s : non trouvé\n", name);
            return 0;
        }
    }

    return 1;
}

int type(int nb,char* args[]){

    if(nb < 2)
        return 0;
        
    short error = 0;
    for(int i=1; i<nb; i++){
        if( !get_type(args[i]) ) {
            error = 1;
            fprintf(stderr, "type: Command '%s' not found.\n", args[i]);
        }
    }
    
    return error;
}
