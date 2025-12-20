#ifndef TREE_H
#define TREE_H

#include "list.h"

enum type_of_next {
    NXT, AND, OR
};

struct cmd_inf {
    char **argv;
    char *infile;
    char *outfile;
    int append;
    int backgrnd;
    struct cmd_inf *psubcmd;
    struct cmd_inf *pipe;
    struct cmd_inf *next;
    enum type_of_next type;
};

typedef struct cmd_inf *tree;
typedef struct cmd_inf node;

void print_tree(tree t, int shift);
tree build_tree(StringList *list);
void clear_tree(tree t);

#endif