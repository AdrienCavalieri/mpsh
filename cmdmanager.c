#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>

#include "cmdmanager.h"
#include "variable.h"
#include "mpsh.h"

#include "int/echo.h"
#include "int/cd.h"
#include "int/exit.h"
#include "int/alias.h"
#include "int/unalias.h"
#include "int/umask.h"
#include "int/mkdir.h"
#include "int/pwd.h"
#include "int/type.h"
#include "int/export.h"
#include "int/history.h"
#include "autocomplet.h"


#include <sys/stat.h>
#include <sys/types.h>


static const char* internal_cmd[] =
    {"echo", "cd", "exit", "alias", "unalias", "umask", "mkdir","pwd", "type", "export", "history", "completion", NULL};
static const int int_cmd_nb = sizeof(internal_cmd)/sizeof(internal_cmd[0]);


#define MAX_ARGS 32
static char* args[32];



/*********************************/
/*            Search             */
/*********************************/


/*********************/
/*      Fullname     */
/*********************/

/*
 * Construit le chemin abolue a partir du nom du d'un fichier
 * et du chemin abolue de son repertoire (Le '/' final est optionnel).
 * malloc(3) est effectue, l'appel a free(3) est necessaire si 
 * le resultat n'est plus utilise.
 */
static char* construct_path(const char* dir_path, const char* file_name) {

    int d_path_len = strlen(dir_path);

    int res_len = d_path_len + strlen(file_name) + 1;
    
    if( dir_path[d_path_len-1] != '/' )
        res_len++;   

    char* res = malloc( sizeof(char)*res_len );

    if( dir_path[d_path_len-1] != '/' )    
        sprintf(res, "%s/%s", dir_path, file_name);
    else
        sprintf(res, "%s%s", dir_path, file_name);

    return res;
}


/*
 * Cherche la commande name dans le repertoire dont le chemin est path.
 * Si rec est different de zero, cherche recurssivement dans tous les sous-repertoires en plus du
 * repertoire choisi. Le resultat est obtenue a l'aide d'un malloc(3), un free(3) est necessaire
 * si il n'est plus utilise.
 */
static char* searchp_ext_cmd_path(const char* path, const char* name, short rec) {
    if( name == NULL || *name == '\0' )
        return NULL;

    // Variable utlisees pour le parcours du repertoire.
    DIR* dir = opendir(path);

    if( dir == NULL ) {
        fprintf(stderr, "%s: %s\n", strerror(errno), path);
        return NULL;
    }
    
    struct dirent* d = readdir(dir);
    struct stat st;

    // Variable utilisee pour contenir le chemin absolue du fichier.
    char* absolute_path;
    // Variable contenant le resultat.
    char* res;
    
    // Parcours du repertoire.
    while( d != NULL ) {

        // On construit la structure stat du fichier/repertoire.
        absolute_path = construct_path( path, d->d_name);
        if( lstat( absolute_path, &st ) < 0 ) {
            fprintf(stderr, "%s in %s: line %d\n", strerror(errno), __FILE__, __LINE__ );
            exit(1);
        }

        // Si c'est un repertoire. 
        if( S_ISDIR(st.st_mode) ) {
            // On le parcours si rec est non null et si il ne s'agit pas des repertoire "." et "..".
            if( rec && strcmp(".", d->d_name) && strcmp("..", d->d_name) ) {
                
                res = searchp_ext_cmd_path(absolute_path, name, rec);
                // Si res est non null on le renvoie, sinon on continue.
                if(res != NULL) {
                    free(absolute_path);
                    closedir(dir);
                    return res;
                }
            }
        }
        
        // Si un nom de commande commence par pref et le fichier est un executable.
        else if( !strcmp(name, d->d_name) && (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) ) {
            closedir(dir);
            return absolute_path;
        }
    
        free(absolute_path);    
        d = readdir(dir);
    }
    closedir(dir);
    return NULL;
}


/*
 * Cherche le chemin de la commande externe name.
 * Renvoie NULL si la commande name est introuvable et son chemin sinon.
 * Le resultat est obtenue a l'aide d'un malloc(3), un free(3) est necessaire
 * si il n'est plus utilise.
 */
static char* search_ext_cmd_path(const char* name) {
    if( name == NULL || *name == '\0' )
        return NULL;
    
    // On cree une copie de $CHEMIN pour pouvoir
    // modifie sa valeur avec strtok.
    char* chemin = strdup(getenv("CHEMIN"));
    char* path = strtok(chemin,":");

    char* res;
    int path_len;
    
    // Parcours de $CHEMIN.
    while( path != NULL ) {

        path_len = strlen(path);

        // Si le chemin courant se termine par "//", on cherche
        // recursivement dans le repertoire et on supprime un '/'.
        if( path[path_len-1] == '/' && path[path_len-2] == '/') {

            path[path_len-1] = '\0';            
            res = searchp_ext_cmd_path(path, name, 1);

        }
        else
            res = searchp_ext_cmd_path(path, name, 0);
        
        if(res != NULL ) {
            free(chemin);
            return res;
        }

        path = strtok(NULL, ":");
    }

    free(chemin);
    return NULL;
}


short is_cmd(const char* name) {
    int i = 0;
    while( internal_cmd[i] != NULL )
        if( !strcmp(internal_cmd[i++], name) )
            return 1;

    char* ext_path = search_ext_cmd_path(name);
    if( ext_path != NULL) {
        free(ext_path);
        return 2;
    }

    return 0;
}

/*********************/
/*       Prefix      */
/*********************/


char* int_cmd_from_pref(const char* pref, int len, int start) {
    if(start >= int_cmd_nb || pref == NULL || *pref == '\0')
        return NULL;
    
    while (internal_cmd[start] != NULL) {
        
        if ( !strncmp (internal_cmd[start], pref, len) )
            return strdup(internal_cmd[start]);
        start++;
    }
    return NULL;
}


/*
 * Fonction auxiliaire de generate_ext_matches.
 * Parcours le repertoire dont le chemin est path.
 */
static char** generate_ext_matches_aux(const char* path, const char* pref, int len, short rec, char* ext_match[], size_t* size, int* adds) {
    if( pref == NULL )
        return NULL;

    DIR* dir = opendir(path);
    struct dirent* d = readdir(dir);
    struct stat st;

    char* absolute_path;
   
    // Parcours du repertoire.
    while( d != NULL ) {

        absolute_path = construct_path( path, d->d_name);
        if( lstat( absolute_path, &st ) < 0 ) {
            fprintf(stderr, "%s in %s: line %d\n", strerror(errno), __FILE__, __LINE__ );
            exit(1);
        }

        // Si c'est un repertoire 
        if( S_ISDIR(st.st_mode) ) {
            // On le parcours si rec est non null et si il ne s'agit pas des repertoire "." et "..".
            if( rec && strcmp(".", d->d_name) && strcmp("..", d->d_name) )
                ext_match = generate_ext_matches_aux(absolute_path, pref, len, rec, ext_match, size, adds);
        }
        // Si un nom de commande commence par pref et le fichier est un executable.
        else if( !strncmp(pref, d->d_name, len) && (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) ) {

            // S'il faut agrandir le tableau.
            if(*adds == *size){
                (*size) *= 2;
                ext_match = realloc(ext_match, sizeof(char*)*(*size));
            }
            // On l'ajoute.
            ext_match[(*adds)++] = strdup(d->d_name);            

        }
            
        free(absolute_path);    
        d = readdir(dir);
             
    }
    
    closedir(dir);
    return ext_match;
}


char** generate_ext_matches(const char* pref, int len) {
    if(pref == NULL) {

        char** empty = malloc(sizeof(char*)*1);
        empty[0] = NULL;
        return empty;
    }
    
    // Taille de depart du tableau.
    size_t size = 16;
    char** ext_match = malloc(sizeof(char*)*size);
    // Nombre d'elements ajoutes.
    int adds = 0;

    
    // On cree une copie de $CHEMIN pour pouvoir
    // modifie sa valeur avec strtok.
    char* chemin = strdup(getenv("CHEMIN"));
    char* path = strtok(chemin,":");

    int path_len;
    
    // Parcours du path.   
    while( path != NULL ) {

        path_len = strlen(path);

        // Si le chemin courant se termine par "//", on cherche
        // recursivement dans le repertoire et on supprime un '/'.
        if( path[path_len-1] == '/' && path[path_len-2] == '/') {
            path[path_len-1] = '\0';            
            ext_match = generate_ext_matches_aux( path, pref, len, 1, ext_match, &size, &adds);    
        }
        else
            ext_match = generate_ext_matches_aux( path, pref, len, 0, ext_match, &size, &adds);
    
        path = strtok(NULL, ":");
    }

    // On ajoute NULL a la fin.
    
    if(adds == size){
        size *= 2;
        ext_match = realloc(ext_match, sizeof(char*)*size);
    }
    ext_match[adds] = NULL;

    // On efface la copie.
    free(chemin);

    return ext_match;
}



/*********************************/
/*           Execution           */
/*********************************/


/*
 * Fonction auxiliaire de exec_ecternal, execute
 * une commande externe dont l'executable se trouve a l'adresse path.
 */
static void exec_ext_aux(const char* path, char* args[]) {
   
    if( fork() == 0 ) {
        
        // On execute la commande.
        execv(path, args);

        // Si execv ne c'est pas termine normalement alors
        // on affiche un erreur et on arrette le processus fils.
        printf("%s\n", strerror(errno));
        exit(1);
    }

    int return_value = 0;
    wait( &return_value );

    char value[12];
    sprintf(value, "%d", return_value);
    set_var_value("?", value);              
}


/*
 * Execute une commande externe. Renvoie 0 si la commande n'a
 * pas ete trouvee.
 */
static short exec_external(char* args[]) {

    char* res = search_ext_cmd_path(args[0]);
    if(res == NULL) {               
        return 0;
    }

    exec_ext_aux(res, args);
    free(res);
    return 1;
}


/*
 * Execute une commande interne.
 */
short exec_internal(int argc, char* args[]) {

    int res;
    char value[12];

    int (*cmd)(int, char**) = NULL;
    
    if( !strcmp(args[0], "echo") )
        cmd = echo;
 
    else if( !strcmp(args[0], "cd") )
        cmd = cd;       

    else if( !strcmp(args[0], "exit") )
        cmd = exit_mpsh;

    else if( !strcmp(args[0], "alias") )
        cmd = alias;

    else if( !strcmp(args[0], "unalias") )
        cmd = unalias;
   
    else if( !strcmp(args[0], "umask") )
        cmd = umask_mpsh;
    
    else if( !strcmp(args[0], "mkdir") )
        cmd = mkdir_mpsh;
     
    else if( !strcmp(args[0], "pwd") )
        cmd = pwd;

    else if( !strcmp(args[0], "type") )
        cmd = type;
    
    else if( !strcmp(args[0], "export") )
        cmd = export;

    else if( !strcmp(args[0], "history") )
        cmd = history;
    
    else if( !strcmp(args[0], "completion") )
        cmd = completion;

    
    if(cmd == NULL)
        return 0;

    res = (*cmd)(argc, args);
    
    sprintf(value, "%d", res);
    set_var_value("?", value);
    return 1;
}


/*
 * Remplis le tableau des arguments a partir de la commande.
 * Renvoie le nombre d'éléments.
 */
static int fill_args(char* s) {
    char* token = strtok(s, " ");
    int i = 0;

    while( token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    return i;
}


short exec_command(int argc, char* args[]) {


    // On verifie si la commande existe dans le repertoire courant
    // si la commande est specifiee par une chemin.
    char* slash = strrchr(args[0], '/');
    if( slash != NULL ) {
        if( fork() == 0 ) {

            char* path = args[0];
            args[0] = slash+1;
            // On execute la commande.
            execv(path, args);

            // Si execv ne c'est pas termine normalement alors
            // on affiche un erreur et on arrette le processus fils.
            printf("%s\n", strerror(errno));
            exit(1);
        }

        int return_value = 0;
        wait( &return_value );

        char value[12];
        sprintf(value, "%d", return_value);
        set_var_value("?", value);              
        
        return 1;
    }

    char* alias = get_alias_value(args[0]);
    if( alias != NULL )
        args[0] = alias;
    
    if( exec_internal(argc, args) )
        return 1;
    
    if( exec_external(args) )
        return 1;

    return 0;
}


int get_return_value() {
    char* res = get_var_value("?");
    return atoi(res);
}

/*********************************/
/*             Read              */
/*********************************/


/*
 * Cherche une eventuel operateur dans line.
 * S'il y en a, renvoie le pointeur du premier trouve,
 * sinon renvoie NULL;
 */
static char* search_operator(char* line) {

    while(*line != '\0' &&
          !(*line == '&' && *(line+1) == '&') &&
          !(*line == '|' && *(line+1) == '|') &&
          *line != '<' &&
          *line != '>' &&
          !(*line == '2' && *(line+1) == '>') &&
          *line != '|' )
        line++;

    if(*line != '\0')
        return line;

    return NULL;
}


/*
 * Evalue les enchainements de commandes via operateurs conditionnel.
 * op pointe vers le premier caractere de cet operateur dans cmd. 
 */
static void evaluate_conditional(char* cmd, char* op) {
    if(op == NULL || cmd == NULL)
        return;
    
    // On met le charactere pointe par *op a null pour
    // pouvoir evaluer la commande precedant l'operateur.
    char symbol = *op;
    *op = '\0';
    // On efface cet espace s'il existe, pour garder une ligne normalisee.
    if( *(op-1) == ' ' )
        *(op-1) = '\0';

    // On evitera cet esapce s'il existe.
    int space = *(op+1) == ' ';
    
    if( symbol == '&' ) {
        read_command(cmd);

        if( get_return_value() )
            return;
        else
            read_command(op+2+space);
    }
    else if( symbol == '|' && *(op+1) == '|' ) {
        read_command(cmd);
 
        if( !get_return_value() )
            return;
        else
            read_command(op+2+space);
    }
    
}



/*
 * Evalue une redirection de commandes via les operateurs de redirection.
 * op pointe vers le premier caractere de cet operateur dans cmd. 
 * Renvoie la valeur de retour de la redirection.
 */
static int evaluate_redirection(char* cmd, char* op) {
    if(op == NULL || cmd == NULL)
        return 1;

    // On met le charactere pointe par *op a null pour
    // pouvoir evaluer la commande precedant l'operateur.   
    char symbol = *op;
    *op = '\0';

    // On efface cet espace s'il existe, pour garder une ligne normalisee.
    if( *(op-1) == ' ' )
        *(op-1) = '\0';

    // On evitera cet esapce s'il existe.
    int space = *(op+1) == ' ';

    // On recupere et ouvre le fichier(en fonction du symbole).
    char* file = symbol == '2' ? op+2+space : op+1+space;            
    int fd = symbol == '<' ? open(file, O_RDONLY) : open(file, O_WRONLY|O_CREAT|O_TRUNC, 0777);
            
    if( fd == -1 ) {
        fprintf(stderr, "%s\n", strerror(errno));
        return 1;
    }

    int argc = fill_args(cmd);

    if( !is_cmd(args[0]) ) {    
        fprintf(stderr, "Commande inconnue : \"%s\"\n", args[0]);                   
        return 1;
    }
    
    if( fork() == 0) {

        // On choisi le descripteur en fonction du symbol.
        int des;
        if(symbol == '<')
            des = 0;
        else if(symbol == '>')
            des = 1;
        else
            des = 2;

        dup2(fd, des);
        close(fd);

        // On arrete le processus fils.
        exit(!exec_command(argc, args));
    }

    // On verifie la valeur de retour.
    int res = 0;
    wait(&res);

    return res;
}


/*
 * Evalue un tube entre deux commandes via l'operateur '|'.
 * op pointe vers le premier caractere de cet operateur dans cmd. 
 * Renvoie la valeur de retour final de l'enchainement.
 */
static int evaluate_pipe(char* cmd, char* op) {
    if(op == NULL || cmd == NULL)
        return 1;

    // On met le charactere pointe par *op a null pour
    // pouvoir evaluer la commande precedant l'operateur.   
    *op = '\0';

    // On efface cet espace s'il existe, pour garder une ligne normalisee.
    if( *(op-1) == ' ' )
        *(op-1) = '\0';
    
    // On evitera cet esapce s'il existe.
    int space = *(op+1) == ' ';

    // On rempli le tableau d'argument des maintenant pour
    // verifier si la commande existe.
    int argc = fill_args(cmd);    

    if( !is_cmd(args[0]) ) {
        fprintf(stderr, "Commande inconnue : \"%s\"\n", args[0]);
        return 1;
    }

    
    // Premier fork pour ne pas toucher aux descipteurs
    // du processus principale.
    if( fork() == 0 ) {

        // On cree le tube.
        int fd[2];

        if( pipe(fd) < 0 ) {
            fprintf(stderr, "%s\n", strerror(errno));
            exit(1);
        }

        // Second fork, le fils envoie sa sortie à l'entree
        // du processus courant.
        if( fork() == 0 ) {
            
            dup2(fd[1], 1);
            close(fd[0]);
            close(fd[1]);
            
            exit(!exec_command(argc, args));
            
        }
        
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);

        // On interprete la suite.
        read_command( op+1+space );
        
        int res = 0;
        wait(&res);
        if( res )
            exit(res);
        
        // On arrete la processus avec la valeur de retour de la derniere commande.
        exit( atoi(get_var_value("?")) );
          
    }

    // On verifie la valeur de retour.
    int res = 0;
    wait(&res);
    return res;
}


void read_command(char* line) {

    if( *line == '\0' )
        return;

    
    char* op = search_operator(line);
    
    if( op != NULL ) {
        
        if( *op == '&' || ( *op == '|' && *(op+1) == '|' )) {
            evaluate_conditional(line, op);
            return;
        }

        int res = 0;
        if( *op == '>' || *op == '<' || (*op == '2' && *(op+1) == '>') ) 
            res = evaluate_redirection(line, op);
        
        else if( *op == '|' )
            res = evaluate_pipe(line, op);

        char value[12];
        sprintf(value, "%d", res);
        set_var_value("?", value);              

        return;
    }

    
    // Affectation de variable.

    // On tente une affectation, si elle echoue, alors ce n'en est pas une.
    // Si elle reussie, on l'applique et on interprette la suite.
    int pos = set_variable(line);
    if( pos ) {
        if( line[pos] != '\0' )
            read_command( line+pos+1 );
        return;
    }
    
    // Execution de commandes.
    
    int argc = fill_args(line);

    if(!exec_command(argc, args)) {
        printf("Commande inconnue : \"%s\"\n", args[0]);
        set_var_value("?", "1");              
    }
}
