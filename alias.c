#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "alias.h"

enum ALIAS_STATE {ALIAS, ANAME, ARGV, READY};
enum UNALIAS_STATE {UNALIAS, UNAME, DONE};

static void print_alias(void *ptr) {
    if (ptr == NULL)
        return;
    struct AliasPair *alias_ptr = (struct AliasPair *) ptr;
    printf("%s", alias_ptr->alias_name);
    fflush(stdout);
    char **alias_argvp = alias_ptr -> argv;
    while (*alias_argvp) {
        printf(" %s", *alias_argvp++);
        fflush(stdout);
    }
    write(STDOUT_FILENO, "\n", 1);
}

static void clean(void *ptr) {
    struct AliasPair *alias_ptr = (struct AliasPair *) ptr;
    free(alias_ptr->alias_name);
    char **argvp = alias_ptr->argv;
    while (*argvp)
        free(*argvp++);
}

void alias(struct Node **head, char **alias_argv) {
    char **argvp = alias_argv;
    char *alias_name = NULL;
    enum ALIAS_STATE state = ALIAS;
    struct AliasPair alias_pair;
    char **alias_argvp = alias_pair.argv;
    int found = 0;
    /* parse the alias command */
    while (*argvp) {
        if (state == ALIAS) {       // expecting alias command
            assert(strcmp(*argvp, "alias") == 0);
            state = ANAME;
        } else if (state == ANAME) { // expecting alias name
            alias_name = *argvp;
            state = ARGV;
        } else {                    // expecting alias argv or already ready
            if (state == ARGV) {    // expecting alias argv
                if (strcmp(alias_name, "alias") == 0 ||
                    strcmp(alias_name, "unalias") == 0 ||
                    strcmp(alias_name, "exit") == 0) {
                    write(STDERR_FILENO, "alias: Too dangerous to alias that.\n", 36);
                    return;
                }
                struct AliasPair *alias_ptr = find_alias(head, alias_name, NULL);
                if (alias_ptr) {    // if alias_name already exists, free the command first
                    found = 1;
                    char **argvp_tmp = alias_ptr->argv;
                    while (*argvp_tmp)
                        free(*argvp_tmp++);
                    alias_argvp = alias_ptr->argv;
                } else
                    alias_pair.alias_name = strdup(alias_name);
            }
            *alias_argvp++ = strdup(*argvp);
            state = READY;
        }
        argvp ++;
    }
    *alias_argvp = NULL;

    /* post-processing */
    if (state == ANAME) {            // the user simply types "alias"
        print(head, print_alias);
    } else if (state == ARGV) {     // the user types "alias [word]"
        print_alias(find_alias(head, alias_name, NULL));
    } else if (state == READY) {    // the user types "alias [word] [command]"
        if (found == 1)
            return;
        push(head, 0, &alias_pair, sizeof(alias_pair));
    } else {
        write(STDERR_FILENO, "error!\n", 7);
    }
}

void unalias(struct Node **head, char **unalias_argv) {
    char **argvp = unalias_argv;
    char *alias_name = NULL;
    enum UNALIAS_STATE state = UNALIAS;
    /* parse the unalias command */
    while (*argvp) {
        if (state == UNALIAS) {     // expecting unalias
            assert(strcmp(*argvp, "unalias") == 0);
            state = UNAME;
        } else if (state == UNAME) { // expecting unalias name
            alias_name = *argvp;
            state = DONE;
        } else {                    // too many arguments
            write(STDOUT_FILENO, 
                "unalias: Incorrect number of arguments.\n", 40);
            return;
        }
        argvp ++;
    }

    /* post-processing */
    if (state == UNAME) {    // the user does not specify the name
        write(STDOUT_FILENO, 
            "unalias: Incorrect number of arguments.\n", 40);
        return;
    } else if (state == DONE) {
        int idx;
        find_alias(head, alias_name, &idx);
        if (idx == -1)
            return;
        erase(head, idx);
    } else {
        write(STDERR_FILENO, "error!\n", 7);
    }
}

struct AliasPair *find_alias(struct Node **head, char *alias_name, int *idx) {
    struct Node *iterator = *head;
    struct AliasPair *alias_ptr;
    if (idx)    *idx = 0;
    while (iterator) {
        alias_ptr = (struct AliasPair *) iterator->data;
        if (strcmp(alias_ptr->alias_name, alias_name) == 0) // found
            return alias_ptr;
        if (idx)    *idx = *idx + 1;
        iterator = iterator->next;
    }
    if (idx)    *idx = -1;
    return NULL;
}

void clear_alias(struct Node **head) {
    clear(head, clean);
}