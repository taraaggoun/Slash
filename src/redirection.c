#include "../include/redirection.h"

///////////////////////////// Fonctions auxilliares /////////////////////////////

// Renvoie une valeur associé au type de redirection et au file descripteur
static int is_redirection(const char * const str, int std_fileno)
{
    if ((strcmp(str, "<" ) == 0))
        return 1;
    if ((strcmp(str, ">" ) == 0) && ((std_fileno == 1) || (std_fileno == 2)))
        return 2;
    if ((strcmp(str, ">|") == 0) && ((std_fileno == 1) || (std_fileno == 2)))
        return 3;
    if ((strcmp(str, ">>") == 0) && ((std_fileno == 1) || (std_fileno == 2)))
        return 4;
    return 0;
}

// Ouvre le fichier fic avec les bon droit et flag
static int get_file_descriptor(const char * const redir_symbol, const char * const fic) 
{
    int redirectionVal = 0;
    if (redir_symbol[0] == '2') {
        redirectionVal = is_redirection(redir_symbol + 1, STDERR_FILENO);
    }
    else 
        redirectionVal = is_redirection(redir_symbol, STDOUT_FILENO);
    int fd = 0;
    switch (redirectionVal) {
        case 1: // <
            fd = open(fic, O_RDONLY);
            break;
        case 2: // > OU 2>
            fd = open(fic, O_WRONLY | O_CREAT | O_EXCL, 0664);
            break;
        case 3: // >| OU 2>|
            fd = open(fic, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            break;
        case 4: // >> OU 2>>
            fd = open(fic, O_WRONLY | O_CREAT | O_APPEND, 0644);
            break;
        default:
            return -1;
    }
    return fd;
}

// Gere les rediraction
static int open_and_dup(int nb_args, char **args, int std_fileno, int i ,int j, int nb_cmds)
{
    int fd = -1;
    int red = 0;
    // pour ignorer 2 dans la redirection
    if (std_fileno - 1 == 1) {// std_fileno - 1 == 1 si std_fileno = STDERR_FILENO
        if (args[i][0] == '2')
            red = is_redirection(args[i] + 1, std_fileno);
    } else
        red = is_redirection(args[i], std_fileno);

    if(nb_cmds != 1 && ((j != 0 && red == 1) || 
    (j != nb_cmds - 1 && std_fileno == STDOUT_FILENO && (red != 0 && red != 1)))) {
        dprintf(STDERR_FILENO, "syntax error \n");
        return -3;
    }
    if (red != 0) {
        //open le bon fichier avec les bon droits
        fd = get_file_descriptor(args[i], args[i + 1]);
        //dup
        if (fd != -1) {
            if (dup2(fd, std_fileno) == -1) {
                dprintf(STDERR_FILENO, "Duplication error.");
                close(fd);
                exit(1);
            } else {
                remove_tab(args, nb_args, i + 1);
                remove_tab(args, nb_args, i);
            }
        } else {
            dprintf(STDERR_FILENO, "Syntax error: open a echoué\n");
            return -2;
        }
    }
    return fd;
}

static int redirection(int nb_args, char **args, int std_fileno, int i, int nb_cmds)
{
    int j = nb_args - 1;
    while (j >= 0) {
        int fd = open_and_dup(nb_args, args, std_fileno, j, i, nb_cmds);
        if (fd == -2) return -2;
        if (fd != -1)
            return fd;
        j -= 1;
    }
    return -1;
}

// Ferme tout les file_descripors
static void close_file_descriptors(int fd0, int fd1, int fd2)
{
    if(fd0!=-1)
        close(fd0);
    if(fd1!=-1)
        close(fd1);
    if(fd2!=-1)
        close(fd2);
}

// redirige fd dans std_fileno
static void redup(int std_fileno, int fd, int fd0, int fd1, int fd2)
{
    if(dup2(fd, std_fileno) == -1){
        dprintf(STDERR_FILENO, "Duplication error.");
        close_file_descriptors(fd0, fd1, fd2);
        exit(EXIT_FAILURE);
    }
}

// verifie si l'open a echoue
static int open_err(int fd0, int fd1, int fd2, int fd0_, int fd1_, int return_val, int nb_args, char **args)
{
    if (fd2 == -2) {
        if (fd0!=-1) {
            redup(STDIN_FILENO, fd0_, fd0, fd1, fd2);
            close(fd0);
        }
        if (fd1!=-1) {
            redup(STDOUT_FILENO, fd1_, fd0, fd1, fd2);
            close(fd1);
        }
        delet_tab(nb_args, args);
        return 1;
    }
    return return_val;
}

// diminue le nombre d'arguments quand les redirections sont faites
static int decr_nb_args(int fd, int nb_args)
{
    if (fd != -1)
        return nb_args-2;
    return nb_args;
}

// Ferme les tout les descripteur de i a nb_cmds
static void close_tube_i(int nb_cmds, int i, int *tube)
{
    for (int j = i; j < nb_cmds; j++) {
        close(tube[2 * j]);
        close(tube[2* j + 1]);
    }
}

static void close_erreur(int i, int fd1, int fd2)
{  
    if (i != 0) {
        close(fd1);
        close(fd2);
    }
}

//////////////////////////// Fonctions principales ////////////////////////////

int redirection_manager(int nb_cmds, char **cmds, int nb_args, char **args, int return_val)
{
    int tube[nb_cmds-1][2];

    for (int i = 0; i < nb_cmds-1; i++) {
        if (pipe(tube[i]) == -1) {
            dprintf(STDERR_FILENO, "Pipe error\n");
            exit(EXIT_FAILURE);
        }
    }
    
    for (int i = 0; i < nb_cmds; i++) {
        nb_args = init_tab(cmds[i], args, ' '); // Tableau d'arguments
        if (nb_args <= 0) { // Si argc = 0: rien a executer 
            close_tube_i(nb_cmds,0,(int*) tube);
            if (nb_args == -1)
                return -1;
            continue;
        }
        nb_args = joker_management(nb_args, args);
        if (nb_args == -2) {
            close_tube_i(nb_cmds,i + 2,(int*) tube);
            exit(EXIT_FAILURE);
        }
        if (nb_args == -1) {
            close_tube_i(nb_cmds,i,(int*) tube);
            break;
        }
        int fd0 = -1;
        int fd1 = -1;
        int fd2 = -1;
        int fd0_ = dup(STDIN_FILENO);
        int fd1_ = dup(STDOUT_FILENO);
        int fd2_ = dup(STDERR_FILENO);

        if (nb_args >= 3) {
            fd0 = redirection(nb_args, args, STDIN_FILENO, i, nb_cmds);
            if (fd0 == -2) {
                close_erreur(i, tube[i - 1][0], tube[i - 1][1]);
                continue;
            }
            if (fd0 == -3) {
                close_tube_i(nb_cmds,i + 2,(int*) tube);
                return_val = 2;
                break;
            } 
            nb_args = decr_nb_args(fd0, nb_args);
            return_val = open_err(-1, -1, fd0, fd0_, fd1_, return_val, nb_args, args);
            if (return_val == 1) {
                close_erreur(i, tube[i - 1][0], tube[i - 1][1]);
                continue;
            }
            fd1 = redirection(nb_args, args, STDOUT_FILENO, i, nb_cmds);
            if (fd1 == -2) {
                close_erreur(i, tube[i - 1][0], tube[i - 1][1]);
                continue;
            }
            if (fd1 == -3) {
                close_tube_i(nb_cmds,i + 2,(int*) tube);
                return_val = 2;
                break;
            }
            nb_args = decr_nb_args(fd1, nb_args);
            return_val = open_err(-1, fd0, fd1, fd0_, fd1_, return_val, nb_args, args);
            if (return_val == 1) {
                close_erreur(i, tube[i - 1][0], tube[i - 1][1]);
                continue;
            }
            fd2=redirection(nb_args, args, STDERR_FILENO , i, nb_cmds);
            if (fd2 == -2) {
                close_erreur(i, tube[i - 1][0], tube[i - 1][1]);
                continue;
            }
            if (fd2 == -3) {
                close_tube_i(nb_cmds,i + 2,(int*) tube);
                return_val = 2;
                break;
            } 
            nb_args = decr_nb_args(fd2, nb_args);
            return_val = open_err(fd0, fd1, fd2, fd0_, fd1_, return_val, nb_args, args);
            if (return_val == 1) {
                close_erreur(i, tube[i - 1][0], tube[i - 1][1]);
                continue;
            }
        }

        if(i!=0){
            dup2(tube[i-1][0], STDIN_FILENO);
            close(tube[i-1][0]);
        }
        
        if(i!= nb_cmds-1){
            dup2(tube[i][1], STDOUT_FILENO);
            close(tube[i][1]);
        }

        return_val = command_manager(nb_args, args, return_val); 

        // redup and close file descriptors
        redup(STDIN_FILENO, fd0_, fd0, fd1, fd2);
        redup(STDOUT_FILENO, fd1_, fd0, fd1, fd2);
        redup(STDERR_FILENO, fd2_, fd0, fd1, fd2);
        close_file_descriptors(fd0, fd1, fd2);
        if (i != 0)
            close(tube[i - 1][1]);
        if (i == nb_cmds)
            close(tube[i][1]);

        delet_tab(nb_args, args); // Liberation de la memoire
    }
    return return_val;
}