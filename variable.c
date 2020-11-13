#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "utils/dictionaryTree.h"
#include "variable.h"

#define NAME_MAX_LEN 32


/**************************/
/*        General         */
/**************************/


int extract(const char* var, variable* container) {
    // On cherche d'abord le '='.
    int i = 0;
    while( !isspace(var[i]) && var[i] != '=' && var[i] != '\0' )
        i++;

    // Si il n'y a pas de '=', si name est vide ou si il y'a un espace autour du '=',
    // on met errno en "Invalid argument" et on renvoie NULL.
    if( i == 0 || isspace(var[i]) || var[i] == '\0' || isspace(var[i+1]) ) {
        errno = EINVAL; 
        return 0;
    }

    // On contruit/remplit name.
    container->name = malloc(sizeof(char)*(i+1));
    if( container->name == NULL ) {
        fprintf(stderr, "%s, file: %s, line: %d\n", strerror(errno), __FILE__, __LINE__);
        exit(1);
    }

    
    strncpy( container->name, var, i );
    container->name[i] = '\0';

   
    // On sauvegarde la position du premier caractere non blancs.
    i++;
    int start = i;

    // On cherche la position de la fin du mot.
    while( var[i] != '\0' && var[i] != ' ' )
        i++;

    
    // On contruit/remplit value.
    int value_len = i-start+1;
    container->value = malloc(sizeof(char)*value_len);
    if( container->value == NULL ) {
        free(container->name);
        fprintf(stderr, "%s, file: %s, line: %d\n", strerror(errno), __FILE__, __LINE__);
        exit(1);
    }

    
    strncpy( container->value, var+start, value_len-1 );
    container->value[value_len-1] = '\0';
        
    return i;
}


d_tree* set_affect(const char* var, d_tree* dict, int* pos) {
     
    // On recupere le nom et la valeur de la variable,
    // on renvoie une exception si il y une erreur.
    variable* container = malloc(sizeof(variable));
    
    *pos = extract(var, container);
    if( !(*pos) ) {
        free(container);
        return NULL;
    }

    char* old = get_value(dict, container->name);
    if( old != NULL )
        free(old);
    
    // On ajoute cette variable a l'arbre.
    dict = add_value(dict, container->name, container->value);
    
    free(container);
    return dict;
}


d_tree* set_value(char* name, const char* content, d_tree* dict) {

    // Ancienne valeur associee a name.
    char* old = get_value(dict, name);

    // Si il n'y a pas de variable avec ce nom, on renvoie false.
    if( old == NULL )
        return NULL;

    // Sinon on libere la memoire utilisee par
    // l'ancienne valeur, et on remplace cette valeur par la nouvelle.
    free(old);
    
    char* copy = malloc(strlen(content)+1); 
    strcpy(copy, content);
       
    return add_value(dict, name, copy);     
}


/**************************/
/*        Variable        */
/**************************/


static d_tree* var_dict = NULL;


int set_variable(const char* var) {
    int pos = 0;
    d_tree* d = set_affect(var, var_dict, &pos);  

    if(d == NULL)
        return pos;
    
    var_dict = d;
    return pos;
}


short set_var_value(char* name, const char* content) {
    d_tree* d = set_value(name, content, var_dict);  

    if(d == NULL)
        return 0;

    var_dict = d;
    return 1;     
}

char* get_var_value(const char* name) {
    return get_value(var_dict, name);
}


short remove_var(const char* name) {
    if( get_value(var_dict, name) == NULL )
        return 0;
    
    var_dict = remove_value(name, var_dict, 1);
    return 1;
}


char* get_var_match_iterator(const char* name, int nth) {
    return get_match_iterator(var_dict, name, nth);
}

