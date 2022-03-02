#include "mysh.h"
#include "linkedList.h"
#include "alias.h"

enum STATE {PRE, IN_ALONE, IN_HEAD, IN_TAIL, IN_MID, POST, DONE};

/* 
 * break the string into small pieces delimitated by space " ", terminated by NULL 
 * returns 1 if command is valid, 0 otherwise
 * if a redirection symbol detected, both the redirection symbol and the file name after 
 *      that is not included in [argv]
 * the file name is stored in [*f_namep]
 */
int parse(char *string, char *argv[], char **f_namep) {
    *f_namep = NULL;
    int idx = 0;
    char *input = string;
    char *token;
    enum STATE state = PRE;
    int count = 0;

    /* the loop for token read in */
    while ((token = strtok(input, " \t\r\n\f\v")) != NULL) {  // read the next token
        if (++count > 99) {
            write(STDERR_FILENO, "too many tokens!\n", 17);
            return 0;
        }
        /* found the redirection symbol '>' */
        char *predir = strchr(token, '>');
        if (predir && state != PRE) {
            write(STDERR_FILENO, "Redirection misformatted.\n", 26);
            return 0;
        }
        if (predir && state == PRE) {
            if (token+strlen(token)-1 != predir && strchr(predir+1, '>')) {
                write(STDERR_FILENO, "Redirection misformatted.\n", 26);
                return 0;
            }
            /* tell the position of '>' in the token */
            if (strlen(token) == 1) {                       // in-alone
                if (idx == 0) {
                    write(STDERR_FILENO, "Redirection misformatted.\n", 26);
                    return 0;
                }
                state = POST;
            } else if (predir == token) {                   // in-head
                if (idx == 0) {
                    write(STDERR_FILENO, "Redirection misformatted.\n", 26);
                    return 0;
                }
                token ++;
                *f_namep = strdup(token);
                state = DONE;
            } else if (token+strlen(token)-1 == predir) {   // in-tail
                *predir = 0;
                argv[idx++] = strdup(token);
                state = POST;
            } else {                                        // in-mid
                *predir = 0;
                argv[idx++] = strdup(token);
                *f_namep = strdup(predir + 1);
                state = DONE;
            }
            if (input != NULL)  input = NULL;
            continue;
        }
        /* not found the '>' symbol */
        if (state == PRE) {
            argv[idx++] = strdup(token);
        } else if (state == POST) {
            *f_namep = strdup(token);
            state = DONE;
        } else {
            write(STDERR_FILENO, "Redirection misformatted.\n", 26);
            return 0;
        }
        /* update strtok input */
        if (input != NULL)  input = NULL;
    }
    if (state != PRE && state != DONE) {
        write(STDERR_FILENO, "Redirection misformatted.\n", 26);
        return 0;
    }
    argv[idx] = '\0';   // null-terminating the array
    if (idx == 0)
        return 0;   // empty line
    return 1;
}

void print_args( char *argv[] ) {
    write(STDOUT_FILENO, "the argv is:\n", 13);
    char **argp = argv;
    while (*argp != NULL) {
        write(STDOUT_FILENO, *argp, strlen(*argp));
        write(STDOUT_FILENO, " ", 1);
        argp ++;
    }
    write(STDOUT_FILENO, "\n\n", 2);
}

void clean_up_in_loop(char **child_argv, char **redir_fname_p, char **command_copy_p) {
    while (*child_argv) {
        free(*child_argv);
        *child_argv = NULL;
        child_argv ++;
    }
    if (*redir_fname_p) {
        free(*redir_fname_p);
        *redir_fname_p = NULL;
    }
    if (*command_copy_p) {
        free(*command_copy_p);
        *command_copy_p = NULL;
    }
}

void clean_up_out_loop(int mode, FILE *fhandle, struct Node **alias_list) {
    if (mode == BATCH)
        fclose(fhandle);
    clear_alias(alias_list);
}