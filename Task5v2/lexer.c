#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>

static int is_special_char(char c) {
    return strchr("|&;><()", c) != NULL;
}

static int is_double_op(const char *p, const char *op) {
    return p[0] == op[0] && p[1] == op[1];
}

void init_list(token_list *list) {
    list->first = NULL;
    list->last = NULL;
}

void add_tok(token_list *list, const char *text) {
    token *t = malloc(sizeof(token));
    t->text = strdup(text);
    t->next = NULL;
    
    if (list->last) {
        list->last->next = t;
        list->last = t;
    } else {
        list->first = list->last = t;
    }
}

void clear_list(token_list *list) {
    token *cur = list->first;
    while (cur) {
        token *next = cur->next;
        free(cur->text);
        free(cur);
        cur = next;
    }
    list->first = list->last = NULL;
}

void print_list(const token_list *list) {
    token *cur = list->first;
    int i = 0;
    printf("=== Tokens ===\n");
    while (cur) {
        printf("[%d]: %s\n", i++, cur->text);
        cur = cur->next;
    }
    printf("==============\n");
}

int tokenize(const char *input, token_list *list) {
    init_list(list);
    if (!input) return 0;
    
    const char *p = input;
    
    char *buf = malloc(64);
    int buf_size = 64;
    int buf_i = 0;
    
    int in_quote = 0;
    int escape = 0;
    int comment = 0;
    
    while (*p && !comment) {
        char c = *p;
        
        if (buf_i >= buf_size - 1) {
            buf_size *= 2;
            char *new_buf = realloc(buf, buf_size);
            if (!new_buf) {
                free(buf);
                clear_list(list);
                return -1;
            }
            buf = new_buf;
        }
        
        if (escape) {
            buf[buf_i++] = c;
            escape = 0;
        } else if (c == '\\') {
            escape = 1;
        } else if (c == '"') {
            in_quote = !in_quote;
            buf[buf_i++] = c;
        } else if (!in_quote && c == '#') {
            comment = 1;
        } else if (!in_quote && isspace(c)) {
            if (buf_i > 0) {
                buf[buf_i] = '\0';
                add_tok(list, buf);
                buf_i = 0;
            }
        } else if (!in_quote && is_special_char(c)) {
            if (buf_i > 0) {
                buf[buf_i] = '\0';
                add_tok(list, buf);
                buf_i = 0;
            }
            
            if (is_double_op(p, "&&")) {
                add_tok(list, "&&");
                p += 2;
                continue;
            } else if (is_double_op(p, "||")) {
                add_tok(list, "||");
                p += 2;
                continue;
            } else if (is_double_op(p, ">>")) {
                add_tok(list, ">>");
                p += 2;
                continue;
            }
            
            char op[2] = {c, '\0'};
            add_tok(list, op);
        } else {
            buf[buf_i++] = c;
        }
        
        p++;
    }
    
    if (buf_i > 0) {
        buf[buf_i] = '\0';
        add_tok(list, buf);
    }
    
    free(buf);
    return 0;
}

void expand_vars(token_list *list) {
    token *cur = list->first;
    while (cur) {
        if (strcmp(cur->text, "$SHOME") == 0) {
            char *v = getenv("HOME");
            if (v) {
                free(cur->text);
                cur->text = strdup(v);
            }
        } else if (strcmp(cur->text, "$SUSER") == 0) {
            char *v = getenv("USER");
            if (!v) {
                v = getlogin();
            }
            if (v) {
                free(cur->text);
                cur->text = strdup(v);
            }
        } else if (strcmp(cur->text, "$SSHELL") == 0) {
            char *v = getenv("SHELL");
            if (v) {
                free(cur->text);
                cur->text = strdup(v);
            } else {
                free(cur->text);
                cur->text = strdup("/bin/sh");
            }
        } else if (strcmp(cur->text, "$SEUID") == 0) {
            uid_t euid = geteuid();
            char tmp[32];
            snprintf(tmp, sizeof(tmp), "%d", (int)euid);
            free(cur->text);
            cur->text = strdup(tmp);
        }
        else if (strcmp(cur->text, "$HOME") == 0) {
            char *v = getenv("HOME");
            if (v) {
                free(cur->text);
                cur->text = strdup(v);
            }
        } else if (strcmp(cur->text, "$USER") == 0) {
            char *v = getenv("USER");
            if (v) {
                free(cur->text);
                cur->text = strdup(v);
            }
        } else if (strcmp(cur->text, "$SHELL") == 0) {
            char *v = getenv("SHELL");
            if (v) {
                free(cur->text);
                cur->text = strdup(v);
            }
        } else if (strcmp(cur->text, "$EUID") == 0) {
            uid_t euid = geteuid();
            char tmp[32];
            snprintf(tmp, sizeof(tmp), "%d", (int)euid);
            free(cur->text);
            cur->text = strdup(tmp);
        }
        
        cur = cur->next;
    }
}