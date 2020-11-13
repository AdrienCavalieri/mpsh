#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "../variable.h"
#include "../cmdmanager.h"
#include "alias.h"


static d_tree* alias_dict = NULL;


int set_alias(char* var) {
    int pos = 0;
    d_tree* d = set_affect(var, alias_dict, &pos);  

    if(d == NULL)
        return pos;
    
    alias_dict = d;
    return pos;
}


short set_alias_value(char* name, char* value) {
    d_tree* d = set_value(name, value, alias_dict);  

    if(d == NULL)
        return 0;

    alias_dict = d;
    return 1;     
}

short remove_alias(char* name) {
    if( get_value(alias_dict, name) == NULL )
        return 0;
    
    alias_dict = remove_value(name, alias_dict, 1);
    return 1;
}

char* get_alias_value(char* name) {
    return get_value(alias_dict, name);
}


char* get_alias_match_iterator(const char* name, int nth) {
    return get_match_iterator(alias_dict, name, nth);
}



int alias(int nb, char* args[]){
   
    if(nb > 1) {
        
        for(int i=1; i<nb; i++)
           
            if( !set_alias(args[i]) ){
                fprintf(stderr, "%s: %s\n", strerror(EINVAL), args[i]);
                return 1;
            }      
    }

    else
        print_all(alias_dict);

    return 0;
}
