#include "../include/tab.h"

//////////////////////////// Fonctions principales ////////////////////////////

int init_tab(char *line, char **tab, char delimit)
{
    memset(tab, 0, MAX_ARGS_NUMBER * sizeof(char *));
    int len_line = strlen(line);
    char tmp[MAX_ARGS_STRLEN] = {0};
    int id = 0;
    int j = 0;

    if(line[len_line - 1] == delimit && delimit != ' '){
        dprintf(STDERR_FILENO, "Syntax error\n");
        return -1;
    }

    for (int i = 0; line[i] != '\0'; i ++) {
        if (line[i] ==  '\\' && i + 1 < len_line && line[i + 1] == delimit) {
            i++;
            tmp[id] = line[i];
            id ++; 
            continue;
        }
        if (line[i] == delimit && (delimit == ' ' || (i > 0 && line[i - 1] == ' ') || i == 0)) {
            if(strcmp(tmp,"") == 0) {
                if(delimit == ' ') continue;
                dprintf(STDERR_FILENO, "Syntax error\n");
                return -1;
            }
            tab[j] = calloc(strlen(tmp) + 1, 1);
            if (tab[j] == NULL) {
                dprintf(STDERR_FILENO, "Erreur malloc\n");
                exit(EXIT_FAILURE);
            }
            strcpy(tab[j],tmp);
            memset(tmp, 0, MAX_ARGS_STRLEN);
            id = 0;
            j ++;
            continue;
        }
        if(id == 0 && line[i] == ' ')
            continue;
        tmp[id] = line[i];
        id ++;
        continue;
    }
    if(strcmp(tmp,"") != 0){    
        tab[j] = calloc(strlen(tmp) + 1, 1);
        if (tab[j] == NULL) {
            dprintf(STDERR_FILENO, "Erreur malloc\n");
            exit(EXIT_FAILURE);
        }
        strcpy(tab[j],tmp);
        j++;
    }
    return j;
}

void delet_tab (int taille, char **tab)
{
    for (int i = 0; i < taille; i++)
        if (tab[i] != NULL) {
            free(tab[i]);
            tab[i] = NULL;
        }
}

int add_tab(int taille, char **commande, char *arg, int i)
{
    if (taille + 1 > MAX_ARGS_NUMBER) {
        dprintf(STDERR_FILENO, "Erreur trop d'argument.");
        return 1;
    }

    for (int j = i; j < taille; j++) {
        char *tmp = commande[j];
        commande[j] = arg;
        arg = tmp;
    }
    return 0;
}

int remove_tab(char **argv, int argc, int i)
{
    if (argc < 1) {
        dprintf(STDERR_FILENO, "Erreur le tableau est vide.");
        return 1;
    }

    free(argv[i]);
    for (int j = i + 1; j < argc; j++) {
        argv[j - 1] = argv[j];
    }
    argv[argc - 1] = NULL;
    return 0;
}