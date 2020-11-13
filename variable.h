#ifndef VARIABLE_H
#define VARIABLE_H

#include "utils/dictionaryTree.h"

typedef struct variable {
    char* name;
    char* value;
} variable;



/**************************/
/*        General         */
/**************************/


/*
 * Extrait le nom et la valeur d'une chaine de caracteres de la forme
 * <nom>=<valeur> respectivement dans "name" et "value" (copies faite avec malloc(3)).
 * Renvoie la position du caractere suivant <valeur> et 0 si il y a une erreur de format,
 * nom vide, pas de '=', (errno EINVAL). 
 */
extern int extract(const char* var, variable* container);

/*
 * Cree ou met a jour une variable a partir d'une chaine
 * de caractere de la forme: <nom>=<valeur> dans dict.
 * Renvoie la position du caractere qui suit <valeur>
 * et 0 si il y a une erreur. 
 */
extern d_tree* set_affect(const char* var, d_tree* dict, int* pos);


/*
 * Modifie la valeur associee a name par content dans dict et renvoie 1.
 * Si il n'y a pas de variable de ce nom, renvoie 0. (Copie de content effectuee). 
 */
extern d_tree* set_value(char* name, const char* content, d_tree* dict);



/**************************/
/*        Variable        */
/**************************/


/*
 * Cree ou met a jour une variable a partir d'une chaine
 * de caracteres de la forme: <nom>=<valeur>.
 * Renvoie la position du caractere qui suit <valeur>
 * et 0 si il y a une erreur. 
 */
extern int set_variable(const char* var);


/*
 * Modifie la valeur associee a name par content et renvoie 1.
 * Si il n'y a pas de variable de ce nom, renvoie 0. 
 */
extern short set_var_value(char* name, const char* content);


/*
 * Cherche la variable de nom "name" et renvoie son contenu.
 * Si cette variable n'existe pas, renvoie NULL.
 */
extern char* get_var_value(const char* name);


/*
 * Supprime la variable de nom name;
 */
extern short remove_var(const char* name);


/*
 * Cherhce une variable ayant name comme prefixe. 
 */
extern char* get_var_match_iterator(const char* name, int nth);
 
#endif
