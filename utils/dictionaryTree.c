#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include "dictionaryTree.h"


static d_tree* create_node(char* name, char* content) {
    d_tree* n = malloc( sizeof(d_tree) );
    
    if( n == NULL ) {
        fprintf(stderr, "%s, file: %s, line: %d\n", strerror(errno), __FILE__, __LINE__);
        exit(1);
    }

    n->name = name;
    n->content = content;

    n->left = NULL;
    n->right = NULL;
    
    return n;
}

static void free_node(d_tree* node, short free_all) {
    if(free_all) {
        free(node->name);
        free(node->content);        
    }
    free(node);
}


d_tree* create_tree(char* rootname, char* rootcontent) {
    return create_node(rootname, rootcontent);
}


static void add_value_aux(d_tree* node, char* name, char* content) {
    int cmp = strcmp(name, node->name);

    // Si on a trouve le noeud, on le met a jour.
    if( cmp == 0 )
        node->content = content;

    // Sinon si name < node->name, on va a gauche.
    else if( cmp < 0 ) {
        
        if( node->left == NULL )
            node->left = create_node(name, content);
        else
            add_value(node->left, name, content);

        //Sinon on va a droite.
    } else {
        if( node->right == NULL )
            node->right = create_node(name, content);
        else
            add_value(node->right, name, content);
    }
}


d_tree* add_value(d_tree* tree, char* name, char* content) {
    // Si name n'est pas valide.
    if( name == NULL || *name == '\0' )
        return NULL;

    // Si l'arbre est vide, on renvoie un noeud.
    if(tree == NULL)
        return create_node(name, content);

    add_value_aux(tree, name, content);
    
    return tree;
}


char* get_value(d_tree* tree, const char* name) {
    if( tree == NULL || name == NULL || *name == '\0' )
        return NULL;

    int cmp = strcmp(name, tree->name);
    
    if( cmp == 0 )       
        return tree->content;
        
    else if( cmp < 0 )
        return get_value(tree->left, name);
    
    else
        return get_value(tree->right, name);

}


d_tree* remove_value(const char* name, d_tree* tree, short free_all) {
    // Si l'arbre est vide on renvoie NULL.
    if( tree == NULL )
        return NULL;

    // On cherche le noeud a supprimer.

    // Variable stockant le pere de la cible a supprimer.
    struct d_tree* targetAnc = NULL;

    // Variable stockant la cible a supprimer.
    struct d_tree* target = tree;

    // boolean premetant de savoir si la cible est un
    // fils gauche ou droit.
    short isLeft = 0;

    int cmp = strcmp(name, target->name );

    /* Recherche du noeud */
    
    while( target != NULL && cmp ) {

        targetAnc = target;
        
        // Si la valeur recherche est inferieur a la cle du noeud actuel,
        // on va a gauche.   
        if( cmp < 0 ) {
            target = target->left;
            isLeft = 1;
        }
        
        // Sinon, on va a droite.
        else {
            target = target->right;
            isLeft = 0;
        }

        if( target != NULL )
            cmp = strcmp(name, target->name );           
    }

    /* Suppression du noeud */
    
    // Si le noeud n'a pas ete trouve.
    if( target == NULL )
        return NULL;

    // Si il n'a pas de fils gauche, alors on change le
    // noeud par son fils droit (foncitonne aussi si les deux sont null).
    if( target->left == NULL ) {

        // Si il s'agit de la racine.
        if( targetAnc == NULL ) {
            struct d_tree* racine = target->right;
            free_node(target, free_all);
            return racine;
        }

        if( isLeft )
            targetAnc->left = target->right;
        else
            targetAnc->right = target->right;

        free_node(target, free_all);
        return tree;
    }

    // Sinon, si il n'a pas de fils droit, alors on change le
    // noeud par son fils gauche.
    else if( target->right == NULL ) {

        // Si il s'agit de la racine.
        if( targetAnc == NULL ) {
            struct d_tree* racine = target->left;
            free_node(target, free_all);
            return racine;
        }

        if( isLeft )
            targetAnc->left = target->left;
        else
            targetAnc->right = target->left;

        free_node(target, free_all);
        return tree;
    }

    // Sinon il a un fils gauche et un fils droit.
    // Dans ce cas on la on echange le noeud avec
    // maximum de son sous-arbre gauche, puis on le supprime.

    // Pere du maximum du sous arbre gauche.
    struct d_tree* maxLeftAnc = target;

    // Maximum du sous arbre gauche.
    struct d_tree* maxLeft = target->left;

    while( maxLeft->right != NULL ) {
        maxLeftAnc = maxLeft;
        maxLeft = maxLeft->right;
    }

    char* tmp = target->name;
    char* tmp2 = target->content;
    target->name = maxLeft->name;
    target->content = maxLeft->content;
    maxLeft->name = tmp;
    maxLeft->content = tmp2;
    
    // Si le maximum n'a pas de fils.
    if( maxLeft->left == NULL ) {
        // Si il sagit de la racine du sous-arbre gauche de la cible.
        if(maxLeftAnc == target) {
            free_node(maxLeftAnc->left, free_all);
            maxLeftAnc->left = NULL;       
        }
        else {
            free_node(maxLeftAnc->right, free_all);
            maxLeftAnc->right = NULL;
        }
        return tree;
    }

    // Sinon on reitere l'operation sur le noeud.
    if(maxLeftAnc == target)
        maxLeftAnc->left = remove_value(name, maxLeft, free_all);   
    else 
        maxLeftAnc->right = remove_value(name, maxLeft, free_all);

    return tree;
}


/*
 * Fonction auxiliaire (recursive) de get_match_iterator.
 */
static char* get_match_iterator_aux(d_tree* tree, const char* name, int* skip) {
    if( tree == NULL || name == NULL || *name == '\0' )
        return NULL;

    // On compare le n premiers caracteres (n etant la taille de name) du nom
    // du noeud avec name. 
    int cmp = strncmp(name, tree->name, strlen(name));

    // Si les 2 sont egaux.
    if( cmp == 0 ) {
        if( !(*skip) )
            return strdup(tree->name);
        else {
            (*skip)--;
            char* res_left = get_match_iterator_aux(tree->left, name, skip);
            if(res_left != NULL)
                return res_left;
            else
                return get_match_iterator_aux(tree->right, name, skip);
        }
    }
    // Si name < tree->name, on va a gauche.
    else if( cmp < 0 )
        return get_match_iterator_aux(tree->left, name, skip);

    // Sinon on va a droite.
    else
        return get_match_iterator_aux(tree->right, name, skip);

}


char* get_match_iterator(d_tree* tree, const char* name, int iter) {
    return get_match_iterator_aux(tree, name, &iter); 
}


void print_all(d_tree* dict) {
    if(dict == NULL)
        return;
    
    printf("%s=%s\n", dict->name, dict->content);
    print_all(dict->left);
    print_all(dict->right);
}
