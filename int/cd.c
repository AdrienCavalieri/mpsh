#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../variable.h"
#include "cd.h"


int cd(int nb, char *args[])
{
  // S'il y a trop d'arguments ...
  if (nb > 2)
  {
    errno = E2BIG;
    fprintf(stderr, "cd: %s\n", strerror(errno));
    return 1;
  }

  char *path = NULL;

  // Si le chemin n'est pas précisé en argument ...
  if (nb == 1) {
    path = getenv("HOME"); // Répertoire utilisateur par défaut.
    if( path == NULL ) {
        fprintf(stderr, "cd: $HOME not set.\n");
        return 1; 
    }
    if( *path == '\0')
        return 0;
        
  }else // Sinon ...
    path = args[1];

  if (chdir(path) < 0)
  {
    fprintf(stderr, "cd: %s\n", strerror(errno));
    return 1; 
  }

  return 0;
}
