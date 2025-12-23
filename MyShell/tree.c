#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "tree.h"
#include "lexer.h"

jmp_buf start_buf;
token_list *token_list_ptr;
token *cur_tok;

static token *get_tok(void);
static token *peek_tok(void);

tree *new_node(void) {
    tree *node = (tree *)malloc(sizeof(tree));
    if (!node) return NULL;
    
    node->argv = NULL;
    node->infile = NULL;
    node->outfile = NULL;
    node->background = 0;
    node->type = NEXT;
    node->append = 0;
    node->pipe = NULL;
    node->next = NULL;
    
    return node;
}

void init_node(tree *node) {
    if (!node) return;
    node->argv = NULL;
    node->infile = NULL;
    node->outfile = NULL;
    node->background = 0;
    node->type = NEXT;
    node->append = 0;
    node->pipe = NULL;
    node->next = NULL;
}

void err(char *msg, char *token) {
    fprintf(stderr, "Error: %s. Token: '%s'\n", msg, token);
    longjmp(start_buf, 1);
}

int is_op(token *t) {
    if (!t || !t->text) return 0;
    return strcmp(t->text, "|") == 0;
}

int is_sep(token *t) {
    if (!t || !t->text) return 0;
    return strcmp(t->text, ";") == 0 || 
           strcmp(t->text, "&") == 0 ||
           strcmp(t->text, "&&") == 0 ||
           strcmp(t->text, "||") == 0;
}

int is_redirect(token *t) {
    if (!t || !t->text) return 0;
    return strcmp(t->text, "<") == 0 || 
           strcmp(t->text, ">") == 0 ||
           strcmp(t->text, ">>") == 0;
}

void set_input(tree *node, char *filename) {
    if (node && filename) {
        if (node->infile) free(node->infile);
        node->infile = strdup(filename);
    }
}

void set_output(tree *node, char *filename) {
    if (node && filename) {
        if (node->outfile) free(node->outfile);
        node->outfile = strdup(filename);
        node->append = 0;
    }
}

void set_output_append(tree *node, char *filename) {
    if (node && filename) {
        if (node->outfile) free(node->outfile);
        node->outfile = strdup(filename);
        node->append = 1;
    }
}

void set_bg(tree *node) {
    if (node) node->background = 1;
}

static token *get_tok(void) {
    token *t = cur_tok;
    if (cur_tok) {
        cur_tok = cur_tok->next;
    }
    return t;
}

static token *peek_tok(void) {
    return cur_tok;
}

tree *parse_simple(void) {
    tree *node = new_node();
    if (!node) return NULL;
    
    init_node(node);
    
    char **argv = malloc(sizeof(char *) * 2);
    int argc = 0;
    int argv_size = 2;
    
    if (!argv) {
        free(node);
        return NULL;
    }
    
    argv[0] = NULL;
    
    token *t = peek_tok();
    
    if (!t || !t->text || 
        is_op(t) || is_sep(t) || is_redirect(t)) {
        free(node);
        free(argv);
        err("Expected command", t ? t->text : "NULL");
        return NULL;
    }
    
    while (t && t->text && 
           !is_op(t) && !is_sep(t) && !is_redirect(t)) {
        
        if (argc >= argv_size - 1) {
            argv_size *= 2;
            char **new_argv = realloc(argv, sizeof(char *) * argv_size);
            if (!new_argv) {
                for (int i = 0; i < argc; i++) free(argv[i]);
                free(argv);
                free(node);
                return NULL;
            }
            argv = new_argv;
        }
        argv[argc++] = strdup(t->text);
        argv[argc] = NULL;
        
        get_tok();
        t = peek_tok();
    }
    
    while (t && is_redirect(t)) {
        char *op = t->text;
        get_tok();
        
        token *filename_tok = get_tok();
        if (!filename_tok || !filename_tok->text) {
            for (int i = 0; i < argc; i++) free(argv[i]);
            free(argv);
            free(node);
            err("Expected filename after", op);
            return NULL;
        }
        
        char *filename = filename_tok->text;
        
        if (strcmp(op, "<") == 0) {
            set_input(node, filename);
        } else if (strcmp(op, ">") == 0) {
            set_output(node, filename);
        } else if (strcmp(op, ">>") == 0) {
            set_output_append(node, filename);
        }
        
        t = peek_tok();
    }
    
    node->argv = argv;
    
    return node;
}

tree *parse_cmd_base(void) {
    token *t = peek_tok();
    
    if (t && strcmp(t->text, "(") == 0) {
        get_tok();
        tree *node = parse_cmd();
        t = get_tok();
        if (!t || strcmp(t->text, ")") != 0) {
            err("Expected ')'", t ? t->text : "NULL");
        }
        return node;
    } else {
        return parse_simple();
    }
}

tree *parse_pipe(void) {
    tree *first = parse_cmd_base();
    if (!first) return NULL;
    
    token *t = peek_tok();
    tree *current = first;
    
    while (t && is_op(t)) {
        get_tok();
        
        tree *right = parse_cmd_base();
        if (!right) {
            err("Expected command after '|'", "NULL");
            return first;
        }
        
        current->pipe = right;
        current = right;
        t = peek_tok();
    }
    
    return first;
}

tree *parse_list(void) {
    tree *first = parse_pipe();
    if (!first) return NULL;
    
    token *t = peek_tok();
    tree *current = first;
    
    while (t && (strcmp(t->text, "&&") == 0 || strcmp(t->text, "||") == 0)) {
        char *op = t->text;
        get_tok();
        
        tree *right = parse_pipe();
        if (!right) {
            err("Expected command after operator", op);
            return first;
        }
        
        current->next = right;
        if (strcmp(op, "&&") == 0) {
            current->type = AND;
        } else if (strcmp(op, "||") == 0) {
            current->type = OR;
        }
        
        current = right;
        t = peek_tok();
    }
    
    return first;
}

tree *parse_cmd(void) {
    tree *node = parse_list();
    if (!node) return NULL;
    
    token *t = peek_tok();
    
    if (!t) return node;
    
    if (strcmp(t->text, "&") == 0) {
        get_tok();
        
        tree *current = node;
        while (current) {
            current->background = 1;
            tree *pipe_cmd = current->pipe;
            while (pipe_cmd) {
                pipe_cmd->background = 1;
                pipe_cmd = pipe_cmd->pipe;
            }
            current = current->next;
        }
        
        tree *next_cmd = parse_cmd();
        if (next_cmd) {
            tree *last = node;
            while (last->next) {
                last = last->next;
            }
            last->next = next_cmd;
            last->type = NEXT;
        }
    }
    else if (strcmp(t->text, ";") == 0) {
        get_tok();
        
        tree *next_cmd = parse_cmd();
        if (next_cmd) {
            tree *last = node;
            while (last->next) {
                last = last->next;
            }
            
            last->next = next_cmd;
            last->type = NEXT;
        }
    }
    return node;
}

tree *build_tree(token_list *lst) {
    if (!lst || !lst->first) return NULL;
    
    token_list_ptr = lst;
    cur_tok = lst->first;
    
    if (setjmp(start_buf) == 0) {
        return parse_cmd();
    } else {
        return NULL;
    }
}

void print_tree(tree *node, int level) {
    if (!node) return;
    
    for (int i = 0; i < level; i++) printf("  ");
    printf("Command at level %d:\n", level);
    
    for (int i = 0; i < level; i++) printf("  ");
    printf("  Args: ");
    if (node->argv) {
        for (int i = 0; node->argv[i]; i++) {
            printf("%s ", node->argv[i]);
        }
    }
    printf("\n");
    
    for (int i = 0; i < level; i++) printf("  ");
    printf("  Input file: %s\n", node->infile ? node->infile : "(none)");
    
    for (int i = 0; i < level; i++) printf("  ");
    printf("  Output file: %s (append=%d)\n", 
           node->outfile ? node->outfile : "(none)", node->append);
    
    for (int i = 0; i < level; i++) printf("  ");
    printf("  Background: %d\n", node->background);
    
    for (int i = 0; i < level; i++) printf("  ");
    printf("  Link type: ");
    switch(node->type) {
        case NEXT: printf("NEXT (;)\n"); break;
        case AND: printf("AND (&&)\n"); break;
        case OR: printf("OR (||)\n"); break;
    }
    
    if (node->pipe) {
        for (int i = 0; i < level; i++) printf("  ");
        printf("  Pipe to:\n");
        print_tree(node->pipe, level + 1);
    }
    
    if (node->next) {
        for (int i = 0; i < level; i++) printf("  ");
        printf("  Next command (type: ");
        switch(node->type) {
            case NEXT: printf(";"); break;
            case AND: printf("&&"); break;
            case OR: printf("||"); break;
        }
        printf("):\n");
        print_tree(node->next, level + 1);
    }
}

void print_node(tree *node) {
    printf("\n=== Command tree structure ===\n");
    print_tree(node, 0);
    printf("=========================\n");
}

void free_tree(tree *node) {
    if (!node) return;
    
    if (node->pipe) free_tree(node->pipe);
    if (node->next) free_tree(node->next);
    
    if (node->argv) {
        for (int i = 0; node->argv[i]; i++) {
            free(node->argv[i]);
        }
        free(node->argv);
    }
    
    if (node->infile) free(node->infile);
    if (node->outfile) free(node->outfile);
    
    free(node);
}