#ifndef ALIAS_H
#define ALIAS_H

#include "linkedList.h"

struct AliasPair {
    char *alias_name;
    char *argv[98];
};

/* add an alias pair to the list headed by [head], or print aliases
 * [alias_argv] should be in the format of the following three scenarios:
        1. {"alias", alias_name, alias_argv[]} --> add alias pair to the list
        2. {"alias", alias_name} --> print the alias pair specified by [alias_name]
        3. {"alias"} --> print all the alias pairs in the list
    otherwise error!

 * new memory would be dynamically allocated for alias_name and argv
 */
void alias(struct Node **head, char **alias_argv);

/* remove the alias pair from the list headed by [head]
 * [unalias_argv] should be in the format of {"unalias", alias_name},
        otherwise error!
 * if [alias_name] is not found in the list, return
 */
void unalias(struct Node **head, char **unalias_argv);

/* find the alias pair that contains [alias_name], and return its pointer
        at the same time, modify the content ponited to by [idx] so that 
        it stores the index of the found item in the list
 * if [alias_name] is not found in the list [head], set [*idx] to -1, and return NULL

 * if the input [idx] is NULL, then ignore the index counting
 */
struct AliasPair *find_alias(struct Node **head, char *alias_name, int *idx);

/* find the alias pair that contains [alias_name], and return its index
 * if [alias_name] is not found in the list [head], return -1
 */

/* free all memories occupied by alias linked list
 */
void clear_alias(struct Node **head);

#endif