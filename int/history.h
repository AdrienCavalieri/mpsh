#ifndef HISTORY_H
#define HISTORY_H

/*
 * Permet d'afficher l'historique des commandes.
 */
int history(int nb, char* args[]);

/*
 * Permet d'importer l'historique des commandes.
 */
void import_history();

/*
 * Permet d'exporter l'historique des commandes.
 */
void export_history();

#endif
