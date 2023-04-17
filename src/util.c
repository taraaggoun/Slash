 #include "../include/util.h"

//////////////////////////// Fonctions principales ////////////////////////////

void signal_management(int b) 
{
    struct sigaction sa = {0};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (!b)
        sa.sa_handler = SIG_IGN;
    else 
        sa.sa_handler = SIG_DFL;
    
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

void make_a_prompt(char * const prompt, int return_val)
{ 
    memset(prompt, 0, PROMPT_MAX);
    char env[PATH_MAX]; // Ou est stocke notre PWD
    strcpy(env, getenv("PWD"));
    
    if(return_val == 0) // La couleur est verte
        sprintf (prompt, "%s[%d]%s", GREEN, return_val, BLUE);
    else { // La couleur est rouge
        if (return_val == 255) // Interompu par un signal
            sprintf (prompt, "%s[SIG]%s", RED, BLUE);
        else 
            sprintf(prompt, "%s[%d]%s", RED, return_val, BLUE);
    }

    int balise_len = strlen(RESET); // Taille d'une balise
    int p_len = strlen(prompt); // Taille dans le prompt
    // Taille de ce que on voit dans prompt
    int len_p = p_len - (2 * balise_len); 
    int env_len = strlen (env); // Taille de PWD
    
    if (env_len + len_p + 2 > 30) // Si la taille d'env est trop longue
        sprintf (prompt + p_len, "...%s", env + (env_len - (25 -  len_p)));
    else  
        sprintf (prompt + p_len, "%s", env);

    // Rajout de la balise de fin de couleur est de "$ "
    sprintf (prompt + strlen (prompt), "%s$ ", RESET); 
}