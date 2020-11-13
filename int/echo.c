#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../variable.h"
#include "echo.h"

int echo(int nb,char* args[]){
    int i=1;
    short opt = 0;

    //recuperation dans arg d'un possible argument
    if(nb>1 && args[1][0]=='-' && args[1][1]=='n') {
        i++;
        if(args[1][1]=='n')
            opt = 1;
    }
  
    //on affiche chaque arguments passe a echo

    for(;i<nb;i++){
        if( i > 1 )
            printf(" ");
        
        if(args[i][0]=='$'){
            if(get_var_value(args[i]+1)!=NULL){
                printf("%s", get_var_value(args[i]+1));
            }else if(getenv(args[i]+1)!=NULL){
                printf("%s", getenv(args[i]+1));
            }
        }else{
            printf("%s",args[i]);
        }
    }

    if( !opt ){
        printf("\n");
    }
    return 0;
}
