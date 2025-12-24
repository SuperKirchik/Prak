#ifndef TREE_H
#define TREE_H

#include <setjmp.h>
#include "lexer.h"

typedef enum {NEXT, AND, OR} node_type;

typedef struct Tree {
    char **argv;
    char *infile;
    char *outfile;
    int background;
    node_type type;
    int append;
    struct Tree *pipe;
    struct Tree *next;
} tree;

// Объявляем переменные как extern
extern jmp_buf start_buf;
extern token_list *token_list_ptr;

void err(char *msg, char *token);

void set_input(tree *node, char *filename);
void set_output(tree *node, char *filename);
void set_output_append(tree *node, char *filename);

int is_op(token *t);
int is_sep(token *t);
int is_redirect(token *t);

void set_bg(tree *node);
void init_node(tree *node);

tree *parse_cmd();
tree *parse_list();
tree *parse_pipe();
tree *parse_cmd_base();
tree *parse_simple();

tree *build_tree(token_list *lst);

tree *new_node();
void print_node(tree *node);
void print_tree(tree *node, int level);
void free_tree(tree *node);

#endif