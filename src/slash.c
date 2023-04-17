#include "../include/slash.h"

////////////////////////////// VARIABLES GLOBALES //////////////////////////////

int nb_cmds = 0;
int nb_args = 0;
char *line = NULL;
char *cmds[MAX_ARGS_NUMBER] = {0};
char *args[MAX_ARGS_NUMBER] = {0};

///////////////////////////// Fonction auxilliare /////////////////////////////

static void delet_all()
{
    delet_tab(nb_cmds, cmds);
    delet_tab(nb_args, args);
    free(line);
    line = NULL;
}

//////////////////////////////////// MAIN /////////////////////////////////////

int main (void) {
    rl_outstream = stderr; 
    atexit(delet_all); // free la memoire quand on exit
    signal_management(0); // Gestion des signaux
    int return_val = 0; // Valeur de retour des commande    
    char prompt[PROMPT_MAX];
    
    while (1) {
        make_a_prompt(prompt, return_val); // Initialise prompt
        
        line = readline(prompt);
        if (line == NULL) // Si la ligne est NULL: fin du programme
            return return_val;
        add_history(line);

        nb_cmds = init_tab(line, cmds, '|'); // Tableau de commandes
        if (nb_cmds <= 0){
            if (nb_cmds == -1)
                return_val = 2;
            delet_tab(nb_cmds,cmds);
            free(line);
            line = NULL;
            continue;
        }
        
        return_val = redirection_manager(nb_cmds, cmds, nb_args, args, return_val);
        if (return_val == -1 || return_val == 2) {
            delet_tab(nb_cmds,cmds);
            free(line);
            line = NULL;
            return_val = 2;
            continue;
        }

        // Liberation de la memoire
        delet_tab(nb_cmds,cmds);
        free(line);
        line = NULL;
    }
    return 0;
}