#include "../include/intern_command.h"

///////////////////////////// Fonctions auxilliares /////////////////////////////

// Test si s represente un entier
static int isNumber(const char * const s)
{
    int i = 0;
    if (s[i] == '+' || s[i] == '-') 
        i++;

    for (; s[i] != '\0'; i++) 
        if (isdigit(s[i]) == 0)
            return 0;
    return 1;
}

// Enleve le slash en derniere position si il y en a un
static void supp_slash(char * const arg)
{
    if (arg[strlen(arg) -1] == '/')
        arg[strlen(arg) -1] = '\0';
}

/** Permet de trouver le prédécésseur et l'enlève lorsque c'est ..
 * renvoie 1 il a trouvé un prédécéceur
 * 0 sinon
 */
static int previous(int *take, int j)
{
    for (int i = j; i >= 0; i--) {
        if (take[i] == 1) {
            take[i] = 0;
            return 1;
        }
    }
    return 0;
}

// Nettoie path -> enleve les . et ..
static void clean_path (char *path, char *new_path) {
    memset(new_path,0,PATH_MAX);
    char *tokens[PATH_MAX];
    char *tok = strtok (path, "/");

    int i = 0;
    int len = 0;
    while (tok != NULL) {
        tokens[i++] = tok;
        len ++;
        tok = strtok(NULL, "/");
    }

    //tableau disant si oui ou non on prendra dans la chaine finale le mot i
    int take[len];
    for(int j = 0; j < len; j++) 
        take[j] = 1;

    for(int j = 0; j < len; j++) {
        //si c'est . on le prend pas
        if(!strcmp(tokens[j],"."))
            take[j] = 0;
        
        /** si c'est .. on enleve celui avant 
         * (mais pas celui strictement avant, celui avant au sens prenable)
         * si previous a trouvé un avant, on enlève aussi le ..
         */
        else if (!strcmp(tokens[j], "..")) {
            previous(take, j-1);
            take[j] = 0;
        }
    }

    //on construit la chaine finale
    strcat(new_path, "/");
    for (int j = 0; j < len; j++) {
        if (take[j]) {
            strcat(new_path, tokens[j]);
            strcat(new_path, "/");
        }
    }
    supp_slash(new_path);
}

// Change de repertoire pour aller dans path
static int change_directory(const char * const path) 
{
    if (chdir (path) != -1) {
        setenv("OLDPWD", getenv("PWD"), 1);
        setenv("PWD", path, 1);
        return 0;
    }
    return 1;
}

// Concatene dans newpath le pwd avec path
static void conc_with_pwd(const char * const path, char * const newpath)
{
    memset(newpath, 0, PATH_MAX);
    if (path[0] != '/')
        strcpy(newpath, getenv("PWD"));
    strcat(newpath,"/"); 
    strcat(newpath, path);
}

// Crée le nouveau pwd de façon logique
static int cd_p(const char * const path)
{
    char newpath[PATH_MAX];
    conc_with_pwd(path, newpath);

    char pwd[PATH_MAX];
    memset(pwd, 0, PATH_MAX);   
    realpath (newpath, pwd);

    return change_directory(pwd);
}

// Crée le nouveau pwd de façon logique
static int cd_l(const char * const path) 
{
    char newpath[PATH_MAX];
    conc_with_pwd(path,newpath);

    char pwd[PATH_MAX];    
    clean_path(newpath, pwd);
    if(change_directory(pwd) == 1)
        return cd_p(path);
    return 0; 
}

//////////////////////////// Fonctions principales ////////////////////////////

void exit_command (int taille, char **commande, int return_val)
{
    long long res = return_val; 
    bool err = false;

    if (taille > 2) {
        err = true;
        dprintf(STDERR_FILENO, "%s\n",strerror(errno));
    }

    if (commande[1] != NULL) { //args[1] contient la valeur de retour val
        if (isNumber(commande[1])) //tester si val est un nombre
            res = atoll(commande[1]);
        else {
            dprintf(STDERR_FILENO, "%s\n",strerror(errno));
            err = true;
        }
    }     
    if( err == false)
        exit(res);
}
 
int cd_command(int taille, char **commande) 
{
    if (taille == 1 || (taille == 2 && (strcmp(commande[1], "-P") == 0 || 
                                    strcmp(commande[1], "-L") == 0)))
        return change_directory(getenv("HOME"));

    if ((taille == 2 && strcmp(commande[1], "-") == 0) || 
        (taille == 3 && strcmp(commande[2], "-") == 0 && 
        (strcmp(commande[1], "-P") == 0 || strcmp(commande[1], "-L") == 0))) {

        if (getenv("OLDPWD") == NULL)
            return 1;
        return change_directory(getenv("OLDPWD"));
    }

   if ((taille == 2 && strcmp(commande[1], "-") != 0) || 
       (taille == 3 && strcmp(commande[1], "-L") == 0))
        return cd_l(commande[(taille == 3) + 1]);

    if (taille == 3 && strcmp(commande[1], "-P") == 0)
        return cd_p(commande[2]);
    
    // Mauvais nombre d'argument ou erreur de syntaxe
    dprintf(STDERR_FILENO, "cd: too many arguments\n");
    return 1;
}

int pwd_command(int taille, char **commande)
{
    char pwd[PATH_MAX];
    memset(pwd, 0, PATH_MAX);

    if(taille > 2)
        return 1;

    if (taille == 2 && strcmp(commande[1], "-P") == 0) {
        char *path = getcwd(NULL,0);
        if (path == NULL)
            return 1;

        strcpy(pwd,path);
        dprintf(STDOUT_FILENO, "%s\n", pwd);
        free(path);   
        path = NULL; 
        return 0;
    } 

    if (taille == 1 || (taille == 2 && strcmp(commande[1], "-L") == 0)) {
        char *path = getenv ("PWD");
        if (path != NULL) {
            strcpy(pwd, path);
            dprintf(STDOUT_FILENO, "%s\n", pwd);
            return 0;
        }
        return 1;
    }

    // Mauvais nombre d'argument ou erreur de syntaxe
    dprintf(STDERR_FILENO, "pwd: too many arguments\n");
    return 1;
}