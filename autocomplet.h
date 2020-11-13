#ifndef AUTOCOMPLET_H
#define AUTOCOMPLET_H


/*
 * Ajoute une completion (de fichiers/reperoires) personalisee pour une commande. 
 * S'il une tel completion existe deja elle est remplacee.
 */
extern int completion(int argc, char* args[]);


/*
 * Essaie de completer le mot contenu dans *com compris entre start et end. 
 */
extern char ** fileman_completion (const char *com, int start, int end);

#endif
