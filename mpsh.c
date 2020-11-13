#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "mpsh.h"
#include "variable.h"
#include "autocomplet.h"
#include "cmdmanager.h"
#include "int/exit.h"
#include "int/history.h"


#define BOLD "\001\e[1m\002"
#define GREEN "\001\e[32m\002"
#define BGRD_BLACK "\001\e[40m\002"
#define BLUE "\001\e[34m\002"
#define DEFAULT "\001\e[0m\002"

#define NAME_LEN 64

static char hostname[NAME_LEN];
static char username[NAME_LEN];

static char current_path[PATH_DEFAULT];

static char prompt[NAME_LEN*2+PATH_DEFAULT];


/**********************************/
/*             Prompt             */
/**********************************/

/*
 * Affiche les informations (utilisateur, nom de la machine, chemin).
 */
static void init_prompt() {
    char* invite = get_var_value("INVITE");
    
    assert(invite != NULL);

    int pos = 0;
    char* prpt_pos = prompt;
    
    while( invite[pos] != '\0' ) {
        
        if( invite[pos] == '\\' ) {

            pos++;
            
            switch(invite[pos]) {

            case 'u':
                prpt_pos += sprintf( prpt_pos, "%s", username );
                break;

            case 'h':
                prpt_pos += sprintf( prpt_pos, "%s", hostname );
                break;
                
            case 'p':
                prpt_pos += sprintf( prpt_pos, "%s", current_path );        
                break;
                
            case '$':
                sprintf( prpt_pos, "$" );
                prpt_pos++;
                break;
                
            default:
                sprintf( prpt_pos, "\\%c", invite[pos] );
                prpt_pos += 2;
                break;
            }
        }
        else{
            sprintf( prpt_pos, "%c", invite[pos] );
            prpt_pos++;     
        }
        pos++;
    }
    sprintf( prpt_pos, "%c", ' ' ); 
}


/**********************************/
/*        String conversion       */
/**********************************/


char* home_to_tilde(char* line) {

    char* h = getenv("HOME");

    if( line == NULL || h == NULL )
        return NULL;
    
    char* p = line;
    char* start;
    
    // On cherche home.
    while( *p != *h && *p != '\0' )
        p++;

    // Si home ne se trouve pas dans la chaine.
    if( *p == '\0' )
        return line;

    start = p;
    // On avance dans les chaines path et home
    // jusqu'a ce qu'elles different (ou qu'on arrive a la fin d'une
    // des deux).
    while( *p != '\0' && *h != '\0' && *p == *h ) {
        p++;
        h++;
    }

    // Si on arrive a la fin de home alors home
    // est dans path, on le remplace par '~'.
    if( *h=='\0' ) {
        start[0] = '~';
        strcpy(start+1, p);
    }
    
    return line;
}


char* tilde_to_home(char* line) {

    char* home = getenv("HOME");
    
    if( line == NULL || home == NULL )
        return NULL;

    char* copy = strdup(line);
    char* tilde = strchr(copy, '~');

    if( tilde == NULL ) {
        free(copy);
        return NULL;
    }

    *tilde = '\0';
    
    size_t len = strlen(copy) + strlen(tilde+1) + strlen(home) + 1;
    char* res = malloc(sizeof(char*)*len);

    sprintf(res, "%s%s%s", copy, home, tilde+1);

    free(copy);
    return res;
}


char* normalize(char* s) {
    assert( s != NULL);

    if( *s == '\0' )
        return s;

    // Position ou on ecrit.
    char* addPos = s;

    // Position ou on lit.
    char* copyPos = s;

    // Flag permetant de virifier si on se trouve
    // dans une sequence de carateres blancs.
    short hasSpace = 0;

    // On passe les caracteres blancs du debut.
    while( isspace( *copyPos ) && *copyPos != '\0')
        copyPos++;

    // Si la chaine ne contenait que des caracteres blancs,
    // on renvoie la chaine vide.
    if(*copyPos == '\0') {
        s[0] = '\0';
        return s;
    }

    // On parcours la chaine caractere par caractere.
    while(*copyPos != '\0') {
        
        if( isspace(*copyPos) ) {

            // Si on commence une sequence de caracteres blancs
            // on ajoute le seul espaces qui sera garde.
            if( !hasSpace ) {
                addPos[0] = ' ';
                addPos++;
                hasSpace = 1;
            }
            
            copyPos++;
            
        } else {

            // On place le caractre a la bonne position,
            // et on incremente les deux pointeurs.
            addPos[0] = copyPos[0];
            addPos++;
            copyPos++;
            hasSpace = 0;
        }
    }

    // On ajoute le caractere null a la fin.
    // Si il y avait des blancs a la fin, alors
    // il ont ete reduis a un espace, on remplace
    // cet epace par le caractere null.
    if(addPos[-1] == ' ')
        addPos[-1] = '\0';
    else
        addPos[0] = '\0';

    return s;
}


/**********************************/
/*           Variables            */
/**********************************/

#define DEFAULT_HIST_SIZE 1000
#define SDEFAULT_HIST_SIZE "1000"

static void init_variables() {
    setenv("CHEMIN","/bin//",1);
    set_variable("?=0");    
    set_variable("INVITE="BOLD GREEN BGRD_BLACK"[\\u@\\h"DEFAULT BGRD_BLACK BOLD":" BLUE"\\p]"DEFAULT BOLD"\\$"DEFAULT);
    set_variable("HISTSIZE="SDEFAULT_HIST_SIZE);

    stifle_history(DEFAULT_HIST_SIZE);
}


/**********************************/
/*              Main              */
/**********************************/


int initialize_readline(){
    rl_readline_name = "mpsh readline";

    rl_special_prefixes = "$";
    // explicite la completion souhaitee
    rl_attempted_completion_function = fileman_completion;
    
    return 0;
}


/*
 * Lis le fichier d'initialisation .mpshrc .
 */
static void read_mpshrc() {
    char* home = getenv("HOME");
    int path_len = strlen(home) + strlen(".mpshrc") + 2;
    char* path = malloc(sizeof(char) * path_len);
    sprintf(path, "%s/%s", home, ".mpshrc");

    FILE* file = fopen(path, "r");

    if( file == NULL ) {
        fprintf(stderr, "Can't open %s: %s\n", path, strerror(errno));
        free(path);
        return;
    }
    
    free(path);
    
    char** line = malloc(sizeof(char*));
    *line = NULL;
    size_t len = 0;

    char* tmp;
    while (getline(line, &len, file) != -1) {

        *line = normalize(*line);
                        
        tmp = *line;
        *line = tilde_to_home(*line);
        if(*line == NULL)
            *line = tmp;
        else
            free(tmp);
            
        read_command( *line );
        free(*line);
        *line = NULL;
    }

    free(*line);
    free(line);
}


/*
 * Actions effectuees avant chaque entree.
 */
static void routine() {
    getcwd( current_path, PATH_DEFAULT);
    home_to_tilde(current_path);
    
    init_prompt();

    char* hs = get_var_value("HISTSIZE");
    int val;
    if(hs == NULL)
        val = DEFAULT_HIST_SIZE;
    else
        val = atoi(hs);

    if(val != history_max_entries)      
        stifle_history(val);
}


int main(void) {    

    struct sigaction act = {.sa_handler = SIG_IGN};;
    sigaction(SIGINT, &act, NULL);

    
    home_to_tilde(current_path);
     
    // Initialise les variables stockant le nom de la machine
    // et le nom de m'utilisateur.
    gethostname(hostname, NAME_LEN);
    getlogin_r(username, NAME_LEN);

    init_variables();    
    
    initialize_readline();

    import_history();

    read_mpshrc();

    char* tmp;    

    // Si il s'agit d'un lanement par redirection.
    if(!isatty(STDIN_FILENO)) {
        
        char** line = malloc(sizeof(char*));
        *line = NULL;
        size_t len = 0;
        
        while (getline(line, &len, stdin) != -1) {

            *line = normalize(*line);
                        
            tmp = *line;
            *line = tilde_to_home(*line);
            if(*line == NULL)
                *line = tmp;
            else
                free(tmp);
            
            read_command( *line );
            free(*line);
            *line = NULL;
        }

        free(*line);
        free(line);
    }

    else {

        char* line;
        while(1) {

            routine();
           
            line =  readline(prompt);

            // Si EOF (Ctrl-d).
            if(line == NULL ) {
                printf("\n");
                char* args[] = {"exit"};
                exit_mpsh(1, args);
            }
                
            line = normalize(line);
                          
            if( *line != '\0' )
                add_history(line);

            tmp = line;
            line = tilde_to_home(line);
            if(line == NULL)
                line = tmp;
            else
                free(tmp);
            
            read_command( line );
            free(line);
            
        }
    }

    return 0;
}
