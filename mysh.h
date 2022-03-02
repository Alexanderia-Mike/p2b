#ifndef MYSH_H
#define MYSH_H

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "linkedList.h"

#define INTERACTIVE 0
#define BATCH 1
#define DEBUG 0

int parse(char *string, char *argv[], char **f_namep);
void print_args( char *argv[] );
void clean_up_in_loop(char **child_argv, char **redir_fname_p, char **command_copy_p);
void clean_up_out_loop(int mode, FILE *fhandle, struct Node **alias_list);

#endif