#define _GNU_SOURCE
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>

#include <readline/readline.h>

#include "autocomplet.h"
#include "cmdmanager.h"
#include "variable.h"
#include "int/alias.h"
#include "utils/dictionaryTree.h"


// On utilise un tableau bi-dimensionnel dynamique
// pour stocker les associations commandes/extensions.
#define DEFAULT_SIZE 8

static int size = DEFAULT_SIZE;
// Tableau a deux dimension de chaines de caracteres.
static char*** except = NULL;
static int last = -1;


/*
 * Renvoie l'index du tableau commencant par "cmd", et 
 * -1 s'il n'existe pas.
 */
static int get_index(const char* cmd) {
    for(int i=0; i<last+1; i++) {
        if( !strcmp(cmd, except[i][0]) )
            return i;
    }

    return -1;
}


/*
 * Ajoute un element a un tableau dynamique de chaines de caracteres.
 */
static char** add_to_darray(char** array, int* last, int* size, char* val) {
    (*last)++;
    if( *last == *size ) {
        (*size) *= 2;
        array = realloc(array, sizeof(char*)*(*size));
        if(array == NULL){
            fprintf(stderr, "%s\n", strerror(errno));
            exit(1);
        }
    }
    array[*last] = val;
    return array;
}


/*
 * Ajoute une nouvelle association commande/extensions au tableau 
 * (on termine le tableau cree par NULL).
 * Si l'ajout echoue, renvoie 0 et affiche l'erreur, sinon renvoie 1.
 */
static int add_to_except(const char* cmd, char* vals[], int nb_of_vals) {

    // On creer except si n'est pas initialise.
    if(except == NULL) {
        except = malloc(sizeof(char**)*size);
        if(except == NULL){
            fprintf(stderr, "%s\n", strerror(errno));
            return 0;
        }
    }
    
    // On verifie si la commande n'est pas deja traitee.
    int index = get_index(cmd);
    if( index < 0) {
        index = ++last;
        
        // On agrandit le tableau si il est plein.
        if( index == size ) {
            size *= 2;
            except = realloc(except, sizeof(char**)*size);
            if(except == NULL){
                fprintf(stderr, "%s\n", strerror(errno));
                return 0;
            }         
        }

    } else {
        // On libere la memoire utilisee pour l'ancienne association.
        int i=0;
        while( except[index][i] != NULL )
            free( except[index][i++] );
        free(except[index]);
    }
    
    except[index] = malloc(sizeof(char*)*(nb_of_vals+2));
    if(except[index] == NULL){
        fprintf(stderr, "%s\n", strerror(errno));
        return 0;
    }

    // On ajoute les elements au tableau (en terminant par NULL).
    except[index][0] = strdup(cmd);

    for(int i=1; i < nb_of_vals+1; i++)
        except[index][i] = strdup( vals[i-1] );

    except[index][nb_of_vals+1] = NULL;

    return 1;
}


int completion(int argc, char* args[]) {

    if( argc < 3) {
        fprintf(stderr, "completion: Missing argument.\n");
        return 1;
    }

     if( !is_cmd(args[1]) ) {
         fprintf(stderr, "completion: Command not found: '%s'.\n", args[1]);
         return 1;
     }

     return !add_to_except(args[1], args+2, argc-2);
}


static char* custom_file_generator(const char* path, int num) {

    static char** files = NULL;
    static int index;
    
    // Si il s'agit du premier appel, on cree le tableau des noms acceptes.
    if (num == 0){

        index = 0;

        /* Recuperation des extensions */
        
        // On recupere le nom de la commande.
        char* copy = strdup(rl_line_buffer);
        char* cmd = strtok(copy, " ");

        
        // On recupere les extensions a accepter (on ne recuperere
        // pas le nom de la commande).
        int cmd_index = get_index(cmd);
        char** extensions = cmd_index < 0 ? NULL : except[cmd_index]+1;
        
        free(copy);

        // Reinitialisation des fichiers/repertoires acceptés.
        if( files != NULL ) {
            // Les elements contenues dans files sont
            // liberes par readline.
            free(files);
            files = NULL;
        }
        
        // Si cet commande ne possede pas de completion personalisee,
        // on s'arrete la. 
        if(extensions == NULL)
            return NULL;

        /* Recherche du repertoire a parcourir et du prefixe recherche */
        
        copy = strdup( path );
        char* slash = strrchr(copy, '/');
        char* pref;
        
        DIR* dir;
        if( slash == NULL ) {
            dir = opendir(".");
            pref = copy;
        } else {
            *slash = '\0';
            dir = opendir(copy);
            pref = slash+1;
        }

        int pref_len = strlen(pref);
       
        if( dir == NULL ) {
            fprintf(stderr, "%s: %s\n", strerror(errno), path);
            return NULL;
        }

        /* Creation du tableau dynamique contenant les nom 
           des fichiers/repertoires acceptes */
        
        int f_size = 8;    
        int last_file = -1;
        files = malloc(sizeof(char*)*f_size);
        if(files == NULL){
            fprintf(stderr, "%s\n", strerror(errno));
            exit(1);
        }
        
        char* point;
        char* tmp;

        /* Parcours du repertoire */
        
        struct dirent* d = readdir(dir);
        while( d != NULL ) {

            // Si il n'a pas pref comme prefixe, on l'ignore.
            if( strncmp(pref, d->d_name, pref_len) ) {
                d = readdir(dir);
                continue;
            }
        
            // On verifie si il s'agit d'un repertoire.
            if( d->d_type == DT_DIR ) {
                tmp = malloc(sizeof(char)*(strlen(d->d_name)+2));
                sprintf(tmp, "%s/", d->d_name);
                files = add_to_darray(files, &last_file, &f_size, tmp);
            }
            
            else {
                
                point = strchr(d->d_name, '.');
                if( point != NULL )

                    for(int i=0; extensions[i] != NULL; i++)
                        if( !strcmp( (point+1), extensions[i] ) ) {
                            files = add_to_darray(files, &last_file, &f_size, strdup(d->d_name));
                            break;
                        }
            }
           
            
            d = readdir(dir);
        }

        /* Nettoyage et finalisation */
        
        closedir(dir);
        free(copy);   

        files = add_to_darray(files, &last_file, &f_size, NULL);
                 
    }

    return files != NULL && files[index] != NULL ?  files[index++] : NULL;
}


/*
 * Fonction donnant la num-ieme commande ayant un nom
 * commencant par com.
 */
static char* command_generator(const char *com, int num){
    static int i_intern, i_ext, i_alias, len;
    static char** ext_match = NULL;
    char *completion;
    char* res;
    
    if (num == 0){
        i_intern = 0;
        i_ext = 0;
        i_alias = 0;
        len = strlen(com);

        // Creation du tableau des noms acceptes.

        if( ext_match != NULL ) {
            int i = 0;
            while( ext_match[i] != NULL )
                free(ext_match[i++]);
            free(ext_match);
        }
            
        ext_match = generate_ext_matches(com, len);
        
    }

    // alias.
    if( i_alias >= 0 ) {
        res = get_alias_match_iterator(com, i_alias++);
        if( res != NULL )
            return res;
        else
            i_alias = -1;
    }
        
    // Commandes internes.
    res = int_cmd_from_pref(com, len, i_intern++);
    if( res != NULL )
        return res;

    // Commandes externes.
    while (ext_match[i_ext] != NULL) {
        completion = ext_match[i_ext++];

        if ( !strncmp (completion, com, len) )
            return strdup(completion);
    }    

    return NULL;
}


/*
 * Cherche le prochain mot ayant pref comme prefixe dans la liste des
 * variables internes. 
 * (num correspond au numero de l'appel)
 */
static char* search_intern_var(const char* pref, int num) {

    // Variable statique utilisee pour avoir
    // le numero de l'element a partir duquel on commence a chercher.
    static int i_intern;

    // Si il s'agit du premier appel pour pref, on (re)initialise i_intern.
    if (num == 0)
        i_intern = 0;    

    // On cherche une variable interne commencant par pref.
    char* completion = get_var_match_iterator(pref, i_intern++);

    // Si on trouve une variable ayant pref comme prefixe,
    // on renvoie une copie de son nom avec '$' devant.
    if( completion != NULL ) {
        
        int res_len = strlen(completion)+2;
        char* res = malloc( sizeof(char)*res_len );
        strncpy(res+1, completion, res_len-1);
        res[0] = '$';
        free(completion);
        return res;

    }

    free(completion);
    return NULL;
}


/*
 * Cherche le prochain mot ayant pref comme prefixe dans la liste des
 * variables d'environnement. 
 * (num correspond au numero de l'appel)
 */
static char* search_env_var(const char* pref, int num) {

    // Variables statiques utilisees, repectivement, pour avoir
    // la position a partir de laquelle on commence a chercher et la taille de
    // pref.
    static int i_ext, len;

    // Variable contenant le mot a comparer.
    char *completion;
    // Variable contenant le resultat.
    char* res;
    int res_len;

    // Si il s'agit du premier appel pour pref, on (re)initialise
    // les variables statiques.
    if (num == 0){
        i_ext = 0;
        len = strlen(pref);
    }

    // On parcour le tableau des variables d'environnement.
    while (environ[i_ext] != NULL) {
        completion = environ[i_ext++];

        // $CHEMIN=efl/rgedzbeuzbeub
        //  01234567 
        // Si on trouve un mot ayant pref comme prefixe,
        // on renvoie une copie de mot avec '$' devant.
        if ( !strncmp (completion, pref, len) ) {
            res_len = strchr(completion, '=') - completion + 2;
            res = malloc(sizeof(char) * res_len);            
            strncpy(res+1, completion, res_len-2);
            res[0] = '$';
            res[res_len-1] = '\0';
            return res;
        }
    }
    
    return NULL;
}


/*
 * Cherche le prochain mot ayant com comme prefixe. 
 * (num correspond au numero de l'appel)
 */
static char *var_generator(const char *pref, int num){
    // booleen indiquant si l'on a deja fait le tour
    // des variables internes.
    static short not_internal;
    static int intern_matches;
    
    if( !num )
        not_internal = 0;
        
    // On cherche dans les variables internes.
    if( !not_internal ) {

        char* res = search_intern_var(pref, num);
        if(res != NULL)
            return res;
        else {
            not_internal = 1;
            intern_matches = num;
        }
    }
    // On cherche dans les variables d'environnement.
    return search_env_var(pref, num-intern_matches);
}


/*
 * Essaie de completer le mot contenu dans *com compris entre start et end. 
 */
char ** fileman_completion (const char *com, int start, int end){
    // Completion possible.
    char **matches = (char **)NULL;

    // Si on est au debut de la ligne, on cherche une commande. 
    if (start == 0 || rl_line_buffer[start-2] == '&' || rl_line_buffer[start-2] == '|')      
        matches = rl_completion_matches (com, command_generator);
    
    // Si le mot commence par '$', on cherche une variable.
    else if( com[0] == '$' )
        matches = rl_completion_matches(com+1, var_generator);

    else {
        matches = rl_completion_matches(com, custom_file_generator);           
    }
        
    // On renvoie le mot trouve (par default cherche les fichier et repertoires
    // du repertoire courant).
    return matches;
}
