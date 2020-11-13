#ifndef ALIAS_H
#define ALIAS_H


/*
 * Cree ou met a jour un alias a partir d'une chaine
 * de caracteres de la forme: <nom>=<valeur>.
 * Renvoie la position du caractere qui suit <valeur>
 * et 0 si il y a une erreur. 
 */
extern int set_alias(char* var);


/*
 * Modifie la valeur associee a name par content et renvoie 1.
 * Si il n'y a pas d'alias de ce nom, renvoie 0. 
 */
extern short set_alias_value(char* name, char* value);


/*
 * Cherche l'alias de nom "name" et renvoie son contenu.
 * Si cette alias n'existe pas, renvoie NULL.
 */
extern char* get_alias_value(char* name);


/*
 * Supprime un alias. Renvoie 0 si 
 * aucun alias de nom name n'existe et 1 sinon. 
 */
extern short remove_alias(char* name);


/*
 * Cherhce un alias ayant name comme prefixe. 
 */
extern char* get_alias_match_iterator(const char* name, int nth);


/*
 * Cree ou met a jour des alias a partir de chaines
 * de caracteres de la forme: <nom>=<valeur>.
 * Renvoie 1 et affiche une erreur si l'une des chaines est incorrecte et 0 sinon. 
 */ 
extern int alias(int nb, char* args[]);


#endif
