#include "../include/jokers.h"

///////////////////////////// Fonctions auxilliares /////////////////////////////

// Initialise before et after avec ce qu'il y a avant et apres c dans path
static int get_before_after_c(const char * const path, char * const before, 
                                                char * const after, char c)
{
    memset(before, 0, MAX_ARGS_STRLEN);
    memset(after, 0, MAX_ARGS_STRLEN);
    int bool = 0;
    int id_before = 0;
    int id_after = 0;
    int len = strlen(path);

    for (int i = 0; i < len; i++) {
        if (path[i] == c && bool == 0) {
            bool = 1;
            continue;
        }
        if (bool == 0) {
            before[i] = path [i]; 
            id_before++;
        } else {
            after[i - id_before -1] = path[i];
            id_after++;
        }
    }
    return bool;
}

// Verifie si file correspond 
static int match_file(const char * const file, const char * const path)
{
    if (strstr(path,"*") == NULL)
        return strcmp(file, path) == 0;
    
    char before[MAX_ARGS_STRLEN] = {0};
    memset(before, 0, MAX_ARGS_STRLEN);
    char after[MAX_ARGS_STRLEN];
    memset(after, 0, MAX_ARGS_STRLEN);
    get_before_after_c(path, before, after, '*');
    
    if (strcmp(before, "")) { // l'etoile n'est pas un prefixe
        dprintf(STDERR_FILENO, "Erreur : le joker n'est pas prefixe\n");
        return -1; 
    }
    return (strcmp(file + strlen(file) - strlen(after), after)) == 0;
}

// Met dans new_path -> path/arg
static void concat(const char * const path, char * const new_path,
                                 const char * const arg, int bool)
{
    if (strcmp(".", path) != 0 || bool) {
        if (strcmp("/", path) == 0) 
            strcpy(new_path, "/");
        else {
            strcpy(new_path, path);
            strcat(new_path, "/");
        }
    }
    strcat(new_path, arg);
}

// Renvoie le nombre d'occurence de c dans string
int count_char(char *string, char c) {
    int count = 0;
    for (int i = 0; string[i] != '\0'; i++) {
        if (string[i] == c) {
            count++;
        }
    }
    return count;
}

// Passe tout les /./ ou //
static void skip(char * const before, char * const path, int *b,
              char * const arg, char * const after, int *is_dir)
{
    while(1) {
        if (strcmp(before,".") == 0 || strcmp(before,"") == 0) {
            if (strcmp(before,".") == 0) {
                if (strcmp(path,".") != 0)
                    strcat(path,"/.");
                *b = 1;
            }
            memset(arg,0,strlen(arg));
            memmove(arg,after,strlen(after));
            *is_dir = get_before_after_c(arg,before,after,'/');

            if (count_char(after, '/') == 0) {
                break;
            }
            continue;
        }
        break;
    }
}

// ajoute dans le tableau le resultat de l'expantion
static int _add_tab(int argc, char **argv, const char *after, const char *path,
                                                        const char *arg, int i)
{
    int cmp = 0;
    char *new_path = calloc(strlen(path) + strlen(arg) + strlen(after) + 2, 1);
    if (new_path == NULL) {
        dprintf(STDERR_FILENO, "Erreur malloc\n");
        return -2;
    }
    concat(path, new_path, arg, 0);
    if(cmp == 0) {
        free(argv[i]);
        argv[i] = new_path;
    } else 
        add_tab(argc + cmp + 1, argv, new_path, i + cmp);
    cmp ++;
    return cmp;
}

// appelle stat sur les bon argument
static int _stat(struct stat *st, const char *path, const char *arg) 
{
    char new_path[MAX_ARGS_STRLEN] = {0};
    strcpy(new_path, path);
    strcat(new_path, "/");
    strcat(new_path, arg);
    if (lstat(new_path, st) == -1)
        return -1;
    return 0;
}

// Expension de '*'
static int star(int argc, char **argv, int id, char *path, char *arg)
{
    int b = 0;
    int cmp = 0;
    struct stat st = {0};
    char before[PATH_MAX] = {0};
    char after [PATH_MAX] = {0};
    int is_dir = get_before_after_c(arg, before, after, '/');
    
    skip(before, path, &b, arg, after, &is_dir);

    if (strcmp(before, "") == 0) {
        if (path[0] != '.') {
            int nb_modif = _add_tab(argc, argv, after, path, before, id);
            return (nb_modif < 0) ? nb_modif : cmp + nb_modif;
        }
        return cmp;
    }
    DIR *dirp = opendir(path);
    if(dirp == NULL)
        return -1;
    struct dirent *entry;
    while ((entry = readdir(dirp))) {
        int match = match_file(entry->d_name, before);
        if (match == 2) return -1;
        if (((!strcmp(entry->d_name, ".") && strcmp(before, "."))
        || (!strcmp(entry->d_name,"..") && strcmp(before, ".."))))
            continue;       
        if( _stat(&st, path, entry->d_name) == -1)
            return -1;
        if (match && is_dir && (S_ISDIR(st.st_mode) || S_ISLNK(st.st_mode))) {
            char new_path[MAX_ARGS_STRLEN] = {0};
            concat(path, new_path, entry->d_name, b);
            int nb_modif = star(argc + cmp, argv, id + cmp, new_path, after);                
            if(nb_modif == -1)
                return -1;
            cmp += nb_modif;
        }
        if (match && !is_dir) {
            if(entry->d_name[0] == '.') continue;
            int nb_modif = _add_tab(argc, argv, after, path, entry->d_name, id + cmp);
            if (nb_modif == -1)
                return -1;
            cmp += nb_modif;
        }
    }
    closedir(dirp);
    return cmp;
}

// appelle star dans chaque cas 
static int parcours(int argc, char ** argv, int id, int cmp, int is_directory,
                                           char *after, char *path, char *tmp)
{
    int nb_changement = 0;
    if(!is_directory || !strcmp(after,""))
        nb_changement = star(argc + cmp, argv, id + cmp, path, tmp + 1);
    else
        nb_changement = star(argc + cmp, argv, id + cmp, path, tmp + 3);
    return nb_changement;      
}

 // Expansion de **
int double_star(int argc, char **argv, int id, char *path, char *arg, int bool)
{
    int cmp = 0;
    struct stat st = {0};
    char before[PATH_MAX] = {0};
    char after [PATH_MAX] = {0};
    char arg_copy [PATH_MAX] = {0};
    strcpy(arg_copy,arg);
    int is_dir = get_before_after_c(arg, before, after, '/');
    
    DIR *dirp = opendir(path);
    if (dirp == NULL) 
        return -1;
    struct dirent *entry;
    //gere le path initial 
    if (bool) {
        bool = 0;
        int nb_modif = parcours(argc, argv, id, cmp, is_dir, after, path, arg_copy);
        if(nb_modif < 0)
            return nb_modif;
        cmp += nb_modif;
    }
    //gere les sous dossiers du path initial récursivement
    while ((entry = readdir(dirp)) != NULL ) {
        _stat(&st, path, entry->d_name);
        //ignore tout ce qui n'est pas un repertoire , . et ..
        if(entry->d_name[0] == '.' || !S_ISDIR(st.st_mode)) 
            continue;
        char pwd[PATH_MAX] = {0};
        concat(path, pwd, entry->d_name, 0);
        int nb_modif = parcours(argc, argv, id, cmp, is_dir, after, pwd, arg_copy);
        if(nb_modif < 0)
            return nb_modif;
        cmp += nb_modif;
        nb_modif = double_star(argc + cmp, argv, id + cmp, pwd, arg_copy,bool);   
        if(nb_modif < 0)
            return nb_modif;
        cmp += nb_modif;
    }
    closedir(dirp);
    return cmp;
}

//////////////////////////// Fonction principale ////////////////////////////

int joker_management(int cmd_len, char **cmd)
{
    int nb_changement = 0;
    for (int i = 0; cmd[i] != NULL; i++) {
        nb_changement = 0;
        if (strstr(cmd[i], "**") != NULL) {
            if (cmd[i][0] == '/')
                nb_changement = double_star(cmd_len, cmd, i, "/", cmd[i], 1);
            else
                nb_changement = double_star(cmd_len, cmd, i, ".", cmd[i], 1);
            
        } else if (strstr(cmd[i], "*") != NULL) {
            if (cmd[i][0] == '/')
                nb_changement = star(cmd_len, cmd, i, "/", cmd[i]);
            else
                nb_changement = star(cmd_len, cmd, i, ".", cmd[i]);
            
        }
        if (nb_changement > 0) {
                cmd_len += nb_changement - 1;
                i += nb_changement - 1;
        }
        if (nb_changement < 0) {
            return nb_changement;
        }
    }
    return cmd_len;
}