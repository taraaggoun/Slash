#ifndef TAB_H
#define TAB_H

/////////////////////////////////// Includes ///////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/////////////////////////////////// Defines ///////////////////////////////////

#define MAX_ARGS_NUMBER 4096
#define MAX_ARGS_STRLEN 4096

////////////////////////////////// Fonctions //////////////////////////////////

/* Initialise le tableau avec les argument de la chaine de caractere 
 * séparer par des delimit
 * renvoie le nombre de case utilisé dans argv
 */
int init_tab(char *, char **, char );

// Free le tableau argv de taille argc et line
void delet_tab(int, char **);

// Ajoute dans un tableau un argument a la position i
int add_tab(int, char **, char *, int);

// Enleve dans un tableau le ieme argument
int remove_tab(char **, int, int);

///////////////////////////////////////////////////////////////////////////////

#endif // TAB_H