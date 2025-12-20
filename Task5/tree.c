#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static StringList *plex = NULL;
static int cur_token = 0;

static tree make_cmd() {
    tree t = malloc(sizeof(node));
    t->argv = NULL;
    t->infile = NULL;
    t->outfile = NULL;
    t->append = 0;
    t->backgrnd = 0;
    t->psubcmd = NULL;
    t->pipe = NULL;
    t->next = NULL;
    t->type = NXT;
    return t;
}

static void add_arg(tree t, char *arg) {
    if (t->argv == NULL) {
        t->argv = malloc(2 * sizeof(char*));
        t->argv[0] = strdup(arg);
        t->argv[1] = NULL;
    } else {
        int count = 0;
        while (t->argv[count] != NULL) count++;
        t->argv = realloc(t->argv, (count + 2) * sizeof(char*));
        t->argv[count] = strdup(arg);
        t->argv[count + 1] = NULL;
    }
}

static char* get_token() {
    if (plex == NULL || plex->items == NULL || 
        cur_token >= plex->cur || plex->items[cur_token] == NULL) {
        return NULL;
    }
    return plex->items[cur_token];
}

static void next_token() {
    if (get_token() != NULL) {
        cur_token++;
    }
}

tree build_tree(StringList *list) {
    if (list == NULL || list->items == NULL) {
        return NULL;
    }
    
    plex = list;
    cur_token = 0;
    
    tree s = make_cmd();
    tree c = s;
    
    char *token;
    
    while ((token = get_token()) != NULL) {
        if (strcmp(token, "(") == 0) {
            printf("Error: скобки не поддерживаются\n");
            clear_tree(s);
            return NULL;
        }
        else if (strcmp(token, ")") == 0) {
            next_token();
        }
        else if (strcmp(token, "|") == 0) {
            next_token();
            c->pipe = make_cmd();
            c = c->pipe;
        }
        else if (strcmp(token, ">") == 0) {
            next_token();
            token = get_token();
            if (token == NULL || prov_raz(token)) {
                printf("Error: ожидалось имя файла после '>'\n");
                clear_tree(s);
                return NULL;
            }
            c->outfile = strdup(token);
            c->append = 0;
            next_token();
        }
        else if (strcmp(token, ">>") == 0) {
            next_token();
            token = get_token();
            if (token == NULL || prov_raz(token)) {
                printf("Error: ожидалось имя файла после '>>'\n");
                clear_tree(s);
                return NULL;
            }
            c->outfile = strdup(token);
            c->append = 1;
            next_token();
        }
        else if (strcmp(token, "<") == 0) {
            next_token();
            token = get_token();
            if (token == NULL || prov_raz(token)) {
                printf("Error: ожидалось имя файла после '<'\n");
                clear_tree(s);
                return NULL;
            }
            c->infile = strdup(token);
            next_token();
        }
        else if (strcmp(token, "&") == 0) {
            tree t = s;
            while (t != NULL) {
                t->backgrnd = 1;
                t = t->next;
            }
            next_token();
        }
        else if (strcmp(token, ";") == 0) {
            next_token();
            c->next = make_cmd();
            c->type = NXT;
            c = c->next;
        }
        else if (strcmp(token, "&&") == 0) {
            next_token();
            c->next = make_cmd();
            c->type = AND;
            c = c->next;
        }
        else if (strcmp(token, "||") == 0) {
            next_token();
            c->next = make_cmd();
            c->type = OR;
            c = c->next;
        }
        else {
            add_arg(c, token);
            next_token();
        }
    }
    
    return s;
}

void clear_tree(tree t) {
    if (t == NULL) return;
    
    if (t->argv != NULL) {
        for (int i = 0; t->argv[i] != NULL; i++) {
            free(t->argv[i]);
        }
        free(t->argv);
    }
    
    if (t->infile != NULL) free(t->infile);
    if (t->outfile != NULL) free(t->outfile);
    
    clear_tree(t->psubcmd);
    clear_tree(t->pipe);
    clear_tree(t->next);
    
    free(t);
}

// Функция печати дерева (из tree_.h)
void print_tree(tree t, int shift) {
    if (t == NULL)
        return;
        
    // Печать отступа
    for (int i = 0; i < shift; i++) {
        printf(" ");
    }
    
    // Печать аргументов
    if (t->argv != NULL) {
        printf("argv: ");
        for (int i = 0; t->argv[i] != NULL; i++) {
            printf("%s ", t->argv[i]);
        }
        printf("\n");
    } else {
        printf("argv: NULL\n");
    }
    
    // Печать остальных полей
    for (int i = 0; i < shift; i++) printf(" ");
    printf("infile: %s\n", t->infile ? t->infile : "NULL");
    
    for (int i = 0; i < shift; i++) printf(" ");
    printf("outfile: %s (append: %d)\n", t->outfile ? t->outfile : "NULL", t->append);
    
    for (int i = 0; i < shift; i++) printf(" ");
    printf("background: %d\n", t->backgrnd);
    
    for (int i = 0; i < shift; i++) printf(" ");
    printf("type: ");
    switch (t->type) {
        case NXT: printf("NXT\n"); break;
        case AND: printf("AND\n"); break;
        case OR: printf("OR\n"); break;
    }
    
    // Рекурсивная печать поддеревьев
    if (t->psubcmd != NULL) {
        for (int i = 0; i < shift; i++) printf(" ");
        printf("psubcmd:\n");
        print_tree(t->psubcmd, shift + 2);
    }
    
    if (t->pipe != NULL) {
        for (int i = 0; i < shift; i++) printf(" ");
        printf("pipe:\n");
        print_tree(t->pipe, shift + 2);
    }
    
    if (t->next != NULL) {
        for (int i = 0; i < shift; i++) printf(" ");
        printf("next:\n");
        print_tree(t->next, shift + 2);
    }
}