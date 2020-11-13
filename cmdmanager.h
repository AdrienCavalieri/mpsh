#ifndef CMDMANAGER_H
#define CMDMANAGER_H

/*
 * Renvoie 0 si la commande est introuvable,
 * 1 si c'est une commande interne,
 * 2 si c'est une commande externe.
 */
extern short is_cmd(const char* name);


/*
 * Renvoie une copie du premier nom de commande interne ayant
 * pref comme prefixe (a partir de la commande start).
 * Renvoie NULL si aucune commande ne correspond. 
 * (Comme une copie est renvoyee, free(3) est necessaire 
 * si celle-ci n'est plus utilisee). 
 */
extern char* int_cmd_from_pref(const char* pref, int len, int start);


/*
 * Genere un tableau contenant le nom des commandes externes
 * commencant par pref.  
 * (Le tableau est genere avec malloc(3), free(3) est necessaire 
 * si celui-ci n'est plus utilisee). De meme pour chacune de ses cases.*/
extern char** generate_ext_matches(const char* pref, int len);


/*
 * Execute une commande.
 */
extern short exec_command(int argc, char* args[]);

/*
 * Renvoie la valeur actuel de $? (valeur de retour de la derniere
 * commande utilisee).
 */
extern int get_return_value();


/*
 * Interprete une ligne de commande.
 */
extern void read_command(char* line);

#endif
