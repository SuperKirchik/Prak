#ifndef LEXER_H
#define LEXER_H

typedef struct token {
    char *text;
    struct token *next;
} token;

typedef struct {
    token *first;
    token *last;
} token_list;

void init_list(token_list *list);
void add_tok(token_list *list, const char *text);
void clear_list(token_list *list);
void print_list(const token_list *list);
int tokenize(const char *input, token_list *list);
void expand_vars(token_list *list);

#endif