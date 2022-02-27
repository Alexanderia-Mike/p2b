#include "mysh.h"
#include "linkedList.h"
#include "alias.h"

static inline void prompt(int mode) {
    if (mode == INTERACTIVE)
        write(STDOUT_FILENO, "mysh> ", 6);
}

int main(int argc, char * argv[]) {
    /* tell whether there is a specified batch file */
    FILE *fhandle;
    int mode;
    if (argc == 1) {            // no file specified, read from stdin
        fhandle = stdin;
        mode = INTERACTIVE;
    }
    else if (argc == 2) {       // the file specified
        fhandle = fopen(argv[1], "r");
        mode = BATCH;
        if (fhandle == NULL) {  // file does not exist
            char msg[255] = "Error: Cannot open file ";
            strcat(msg, argv[1]);
            strcat(msg, ".\n");
            write(STDERR_FILENO, msg, strlen(msg));
            return 1;
        }
    }
    else {                      // more than two arguments
        write(STDERR_FILENO, "Usage: mysh [batch-file]\n", 25);
        return 1;
    }
    
    /* the loop for executing the commands */
    prompt(mode);
    char command[513];
    char *command_copy = NULL;
    char *retchar;
    char *child_argv[100];
    char *redir_fname = NULL;
    struct Node **alias_list = newList();
    int too_long_flag = 0;
    while ((retchar = fgets(command, 513, fhandle)) != NULL) {
        too_long_flag = 0;
        if (DEBUG) {
            printf("---------------\n");
            fflush(stdout);
        }
        if (mode == BATCH)              // echo the command if it is batch mode
            write(STDOUT_FILENO, command, strlen(command));
        while (!(command[strlen(command)-1] == '\n' || feof(fhandle))) {
            too_long_flag = 1;
            fgets(command, 513, fhandle);   // the line is too long
            if (DEBUG)
                write(STDOUT_FILENO, "the command is too long! Skipped\n", 33);
            continue;
        }
        if (too_long_flag) {
            write(STDOUT_FILENO, ": command too long!\n", 20);
            continue;
        }
        command_copy = strdup(command);
        if (!parse(command_copy, child_argv, &redir_fname)) {
            prompt(mode);
            continue;                   // parse the command, and skip if whitespace
        }
        if (DEBUG) {
            print_args( child_argv );
            printf("the redir fname is %s\n\n", redir_fname);
            fflush(stdout);
        }
        if (!strcmp(child_argv[0], "alias") ||  // aliasing and unaliasing
            !strcmp(child_argv[0], "unalias")) {
            if (!strcmp(child_argv[0], "alias"))
                alias(alias_list, child_argv);
            else if (!strcmp(child_argv[0], "unalias"))
                unalias(alias_list, child_argv);
            prompt(mode);
            continue;
        }
        struct AliasPair *alias_ptr = find_alias(alias_list, child_argv[0], NULL);
        if (alias_ptr) {    // an alias pair found, replace child_argv
            char **argvp = child_argv;
            while (*argvp)
                free(*argvp++);
            argvp = child_argv;
            char **alias_argvp = alias_ptr->argv;
            while (*alias_argvp) {
                *argvp++ = strdup(*alias_argvp++);
            }
            *argvp = NULL;
        }
        if (!strcmp(command, "exit") || !strcmp(command, "exit\n"))
            return 0;                   // exit
        int retint = fork();            // fork the process to do the command
        
        /* fork fails */
        if (retint < 0) {
            write(STDERR_FILENO, "fork failed\n", 12);
            return 1;
        }
        
        /* the child process */
        else if (retint == 0) {
            char out[1000];
            if (redir_fname) {  // stdout redirection
                FILE *redir_fhandle = fopen(redir_fname, "w");
                if (redir_fhandle == NULL) {
                    strcpy(out, "Cannot write to file ");
                    strcat(out, redir_fname);
                    strcat(out, ".\n");
                    write(STDERR_FILENO, out, strlen(out));
                    prompt(mode);
                    _exit(1);
                }
                dup2(fileno(redir_fhandle), STDOUT_FILENO);
            }
            execv(child_argv[0], child_argv);
            strcpy(out, child_argv[0]);
            strcat(out, ": Command not found.\n");
            write(STDERR_FILENO, out, strlen(out));
            _exit(1);                   // errors in command
        }
        
        /* the parent process */
        else {
            int status;
            waitpid(retint, &status, 0);    // wait for its child
            prompt(mode);
        }
        /* clean up inside the loop */
        char **argp = child_argv;
        while (*argp)
            free(*argp++);
        if (redir_fname)
            free(redir_fname);
        if (command_copy)
            free(command_copy);
    }

    /* clean up outside the loop */
    if (mode == BATCH)
        fclose(fhandle);
    clear_alias(alias_list);
}