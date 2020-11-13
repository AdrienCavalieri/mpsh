#ifndef DICTIONARYTREE_H
#define DICTIONARYTREE_H

struct d_tree {
    char* name;
    char* content;

    struct d_tree* left;
    struct d_tree* right;
};

typedef struct d_tree d_tree;

/*
 * Creee l'arbre dictionnaire à partir des valeurs de la racine.
 */
d_tree* create_tree(char* rootname, char* rootcontent);


/*
 * Creee une nouvelle association nom/contenue si aucun noeud
 * n'a le nom 'name'. Si un noeud a le meme nom alors sont contenue
 * est modifie. Renvoie NULL si 'name' est NULL ou est la chaine
 * de characteres vide. 
 */
d_tree* add_value(d_tree* tree, char* name, char* content); 


/*
 * Supprime un noeud, dont la cle est egale a name,
 * de l'abr passe en parametre. Renvoie l'arbre modifie si l'element
 * existait et NULL sinon.
 * Si free_all est different de 0, alors remove_value liberera l'espace
 * alloue par name et content avec free(3).
 */
d_tree* remove_value(const char* name, d_tree* tree, short free_all);


/*
 * Renvoie la valeur associee a name si elle existe.
 * Renvoie NULL sinon (ou si un des parametres est NULL).
 */
char* get_value(d_tree* tree, const char* name);


/*
 * Cherche le n-ieme mot contenant name comme prefixe et renvoie 
 * une copie ( free(3) necessaire ).
 */
char* get_match_iterator(d_tree* tree, const char* name, int nth);

void print_all(d_tree* dict);


#endif
