#ifndef UTIL_H
#define UTIL_H

/////////////////////////////////// Includes ///////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if __APPLE__
    #include <limits.h> 
    #include <signal.h>
#else
    #include <sys/signal.h>
    #include <linux/limits.h> 
#endif

/////////////////////////////////// Defines ///////////////////////////////////

#define PROMPT_MAX 60

#define BLUE "\001\033[34m\002"
#define RED "\001\033[91m\002"
#define GREEN "\001\033[32m\002"
#define RESET "\001\033[00m\002"

////////////////////////////////// Fonctions //////////////////////////////////

// Gere les signaux
void signal_management(int);

// Initialise le prompt
void make_a_prompt(char * const, int);

///////////////////////////////////////////////////////////////////////////////

#endif // UTIL_H