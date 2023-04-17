#ifndef INTERN_COMMAND_H
#define INTERN_COMMAND_H

/////////////////////////////////// Includes ///////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#if __APPLE__
    #include <limits.h> 
#else
    #include <linux/limits.h> 
#endif

////////////////////////////////// Fonctions //////////////////////////////////

// Exit
void exit_command (int, char **, int);

// Cd
int cd_command (int, char **);

// Pwd
int pwd_command (int, char **);

///////////////////////////////////////////////////////////////////////////////

#endif // INTERN_COMMAND_H