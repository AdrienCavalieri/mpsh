#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "history.h"

#include "../mpsh.h"
#include "../variable.h"
#include "../cmdmanager.h"
#include <readline/readline.h>
#include <readline/history.h>


void import_history()
{
    char *home = getenv("HOME");
    char *path = malloc(sizeof(char)*( strlen(home)+strlen("/.mpsh_history")+1 )); // Chemin du fichier .mpsh_history

    // Définition du chemin de .mpsh_history.
    sprintf(path, "%s/.mpsh_history", home);

    FILE *f = fopen(path, "r");
    free(path);
    
    if (f == NULL)
    {
        fprintf(stderr, "%s\n", strerror(errno));    
        return;
    }

    // Parcourir le fichier .mpsh_history.
    char** line = malloc(sizeof(char*));
    *line = NULL;
    size_t len = 0;
    int reads;
    
    while ( (reads = getline(line, &len, f)) != -1)
    {
        // On supprime le '\n'.
        (*line)[reads-1] = '\0';
        add_history(*line); // Ajouter la ligne à l'historique.
        free(*line);
        *line = NULL;
    }

    free(*line);
    free(line);
    fclose(f);
}


void export_history()
{
    char *home = getenv("HOME");
    char *path = malloc(sizeof(char)*( strlen(home)+strlen("/.mpsh_history")+1 )); // Chemin du fichier .mpsh_history

    // Définition du chemin de .mpsh_history.
    sprintf(path, "%s/.mpsh_history", home);

    FILE *f = fopen(path, "w");
    free(path);

    if (f == NULL)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return;
    }
  
    // Récupérer l'état de l'historique.
    HISTORY_STATE *h_s = history_get_history_state();
    
    // Récupérer la liste de l'historique.
    HIST_ENTRY **h_l = history_list();

    // Ecrire en fin de fichier les commandes de l'historique.
    for (int i = 0; i < h_s->length; i++)
        fprintf(f, "%s\n", h_l[i]->line); // Ajoute la lignes de l'historique à la fin du fichier avec un numéro de ligne.

    fclose(f);
    free(h_s);
}

int history(int nb, char* args[])
{

    if( nb > 2) // S'il y a trop d'arguments ...
     { 
         errno = E2BIG;
         fprintf(stderr, "%s\n", strerror(errno));
         return 1; 
     }
    
    int n = 0;

    // Récupérer l'état de l'historique.
    HISTORY_STATE *h_s = history_get_history_state();
    
    // Récupérer la liste de l'historique.
    HIST_ENTRY **h_l = history_list();

 
    // S'il n'y a qu'un argument (la commande) ...
    if (nb == 1)
    {
        // Afficher les commandes de l'historique numérotées.
        for (int i = 0; i < h_s->length; i++)
            printf("%d %s\n", i, h_l[i]->line);
    }

    // S'il y a 2 arguments (la commande et n ou -n) ...
    else if (nb == 2)
    {
        n = atoi(args[1]);
        
        if (n > 0)
        {
            if( n >= h_s->length )
            {
                fprintf(stderr, "history: command number %d doesn't exists.\n", n);
                return 1;
            }
            
            // On relance la commande n.
            char* home_norm = tilde_to_home(h_l[n]->line);
            if(home_norm != NULL) {
                read_command( home_norm );
                free(home_norm);
            }else
                read_command( h_l[n]->line );    
        }
        
        else
        {
            // On fixe à n le nombre de commandes enregistrées dans l'historique.
            // On multiplie -n par (-1) pour retomber sur n.
            char val[16];
            sprintf(val, "%d", n * (-1));
            set_var_value("HISTSIZE", val);
        }
    }
    
    free(h_s);
    return 0;
}
