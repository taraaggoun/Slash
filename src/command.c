#include "../include/command.h"

///////////////////////////// Fonction auxilliare /////////////////////////////

// Renvoie une valeur associé a la commande interne
static int cmd_value (const char * const cmd)
{
    if (strcmp(cmd, "cd") == 0) 
        return 1;
    if (strcmp(cmd, "pwd") == 0)
        return 2;
    if (strcmp(cmd, "exit") == 0) 
        return 3;
    return 0;
}

//////////////////////////// Fonctions principales ////////////////////////////

int extern_command(int cmd_len, char **cmd)
{
    int ret = 0;
    
    pid_t pid = fork();
    if (pid == -1) { // Si le fork échoue
        dprintf(STDERR_FILENO, "Fork error.");
        exit(EXIT_FAILURE);
    } 
    if (pid == 0){ // Processus fils qui exécute la commande
        signal_management(1);
        if (execvp(cmd[0], cmd) == -1) {
            // Valeur de retour pour une commande inexistante
            dprintf(STDERR_FILENO, "%s\n", strerror(errno));
            exit(errno); 
        }
    }

    /** Processus père qui attend que le fils ait fini avant de renvoyer la
     * valeur de retour
     */
    else {
        wait(&ret);
        if (WIFEXITED(ret)) 
            return WEXITSTATUS(ret);
        return 255;
    }
    return 127;
}

int command_manager(int cmd_len, char **cmd, int return_val)
{
    int cmdVal = cmd_value(cmd[0]);

    switch (cmdVal) {
        case 1: // Cd
            return_val = cd_command(cmd_len, cmd);
            if (return_val == 1)
                if (strcmp(strerror(errno),"Success") != 0)
                    dprintf(STDERR_FILENO, "cd: %s\n",strerror(errno));
            break;
        case 2: // Pwd
            return_val = pwd_command(cmd_len, cmd);
            if (return_val == 1)
                if (strcmp(strerror(errno),"Success") != 0)
                    dprintf(STDERR_FILENO, "cd: %s\n",strerror(errno));
            break;
        case 3: // Exit
            exit_command(cmd_len, cmd, return_val);
            return_val = 1;
            break;
        default: // Commandes externes
            return_val = extern_command(cmd_len, cmd);
            break;
    }
    return return_val;
}