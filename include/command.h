#ifndef COMMAND_H
#define COMMAND_H

/////////////////////////////////// Includes ///////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#if __APPLE__
    #include <wait.h>
    #include <types.h>
#else
    #include <sys/wait.h>
    #include <sys/types.h> 
#endif

#include "util.h"
#include "intern_command.h"

////////////////////////////////// Fonctions //////////////////////////////////

// Execute la commande externe
int extern_command(int, char **);

// Appelle la bonne commande
int command_manager(int, char **, int);

///////////////////////////////////////////////////////////////////////////////

#endif // COMMAND_H