#ifndef MPSH_H
#define MPSH_H

#define PATH_DEFAULT 128

/*
 * Remplace le chemin de HOME par '~' dans la chaine
 * passe en parametre (si il s'y trouve).
 */
extern char* home_to_tilde(char* line);


/*
 * Remplace le '~' par la valeur de HOME.
 * Renvoie NULL si la chaine ne contient par de '~'
 * ou si line est NULL ou si HOME n'existe pas.
 * Sinon revoie une nouvelle chaine avec '~' remplace par
 * HOME, la chaine est obtenue avec malloc(3), donc free(3)
 * est necessaire.
 */
extern char* tilde_to_home(char* line);


/*
 * Supprime tous les caracteres blancs avant et apres 
 * dans la chaine de caracteres donnee, et reduit les autres a un espace.
 * Renvoie la chaine modifiee.
 */
extern char* normalize(char s[]);


#endif

