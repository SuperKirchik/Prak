#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

#define N 16

void clearlist(StringList *list){
    int i;
    if (list->items == NULL) return;
    for (i=0; i < list->cur; i++){
        free(list->items[i]);
        list->items[i] = NULL;
    }
    list->size=0;
    list->cur=0;
    free(list->items);
    list->items = NULL;
}

void nullst(StringList *list){
    list->size=0;
    list->cur=0;
    list->items = NULL;
}

void termlist(StringList *list){
    if(list->items == NULL) return;
    if(list->cur > list->size - 1){
        list->items = realloc(list->items, (list->size+1)*sizeof(*list->items));
    }
    list->items[list->cur] = NULL;
}

void nullbuf(buffer *buff){
    buff->buf=NULL;
    buff->size=0;
    buff->cur=0;
}

void addsym(buffer *buff, char c){
    if (buff->cur >= buff->size) {
        buff->size += N;
        buff->buf = realloc(buff->buf, buff->size);
    }
    buff->buf[buff->cur++]=c;
}

void addword(buffer *buff, StringList *list)
{
    if (buff->cur >= buff->size) {
        buff->size += 1;
        buff->buf = realloc(buff->buf, buff->size);
    }
    buff->buf[buff->cur++]='\0';
    buff->buf = realloc(buff->buf, buff->size = buff->cur);
    if (list->cur >= list->size) {
        list->size += N;
        list->items = realloc(list->items, list->size * sizeof(*list->items));
    }
    list->items[list->cur++] = strdup(buff->buf);
    free(buff->buf);
    buff->buf = NULL;
    buff->size = 0;
    buff->cur = 0;
}

void printlist(StringList *list){
    int i;
    if (list->items==NULL) return;
    for (i=0; list->items[i] != NULL; i++) 
        printf("%s\n",list->items[i]);
}

int prov_spec(char c){
    return c == '>' ||
    c == '|' ||
    c == '&' ||
    c == '<' ||
    c == '(' ||
    c == ';' ||
    c == ')';
}

int symset(char c){
    return c!='\n' &&
    c!=' ' &&
    c!='\t' &&
    prov_spec(c)==0 &&
    c!= '\0' &&
    c!= EOF;
}

int pprov_spec(char c){
    return c == '>' ||
    c == '|' ||
    c == '&';
}

int prov_raz(char *token){
    if (token == NULL) return 0;
    return (strcmp(token, "|") == 0) ||
           (strcmp(token, "||") == 0) ||
           (strcmp(token, ">") == 0) ||
           (strcmp(token, ">>") == 0) ||
           (strcmp(token, "&&") == 0) ||
           (strcmp(token, "&") == 0) ||
           (strcmp(token, ";") == 0) ||
           (strcmp(token, "<") == 0) ||
           (strcmp(token, "(") == 0) ||
           (strcmp(token, ")") == 0);
}

// Замена переменных окружения
void replace_variables(StringList *list) {
    for (int i = 0; list->items[i] != NULL; i++) {
        char *item = list->items[i];
        
        if (item[0] == '$') {
            char *var_name = item + 1;
            char *value = NULL;
            
            if (strcmp(var_name, "SHOME") == 0 || strcmp(var_name, "HOME") == 0) {
                value = getenv("HOME");
            } else if (strcmp(var_name, "SUSER") == 0 || strcmp(var_name, "USER") == 0) {
                value = getenv("USER");
                if (value == NULL) {
                    struct passwd *pw = getpwuid(getuid());
                    if (pw) value = pw->pw_name;
                }
            } else if (strcmp(var_name, "SEUID") == 0) {
                static char euid_str[20];
                snprintf(euid_str, sizeof(euid_str), "%d", geteuid());
                value = euid_str;
            } else if (strcmp(var_name, "SSHELL") == 0 || strcmp(var_name, "SHELL") == 0) {
                value = getenv("SHELL");
            } else {
                value = getenv(var_name);
            }
            
            if (value != NULL) {
                free(list->items[i]);
                list->items[i] = strdup(value);
            }
        }
    }
}

// Проверка синтаксиса
int find_serror(StringList *list){
    int sum = 0;
    int ozhcom = 1;  // 1 - ожидаем команду, 0 - ожидаем разделитель
    int expect_filename = 0;  // Для <, >, >>
    
    for (int i = 0; list->items[i] != NULL; i++) {
        char *token = list->items[i];
        
        if (strcmp(token, "(") == 0) {
            if (!ozhcom) return 1;
            sum++;
        } 
        else if (strcmp(token, ")") == 0) {
            if (ozhcom || sum <= 0) return 1;
            sum--;
            ozhcom = 0;
        } 
        else if (prov_raz(token)) {
            if (expect_filename) {
                return 1;
            }
            
            if (ozhcom) {
                if (i == 0) return 1;
                return 1;
            }
            
            if (strcmp(token, "|") == 0 || 
                strcmp(token, "||") == 0 || 
                strcmp(token, "&&") == 0 || 
                strcmp(token, ";") == 0) {
                ozhcom = 1;
            }
            else if (strcmp(token, "&") == 0) {
                if (list->items[i+1] != NULL && 
                    strcmp(list->items[i+1], ";") != 0) {
                    return 1;
                }
                if (list->items[i+1] != NULL) {
                    i++;
                }
                ozhcom = 1;
            }
            else if (strcmp(token, "<") == 0 || 
                     strcmp(token, ">") == 0 || 
                     strcmp(token, ">>") == 0) {
                if (list->items[i+1] == NULL || prov_raz(list->items[i+1])) {
                    return 1;
                }
                expect_filename = 1;
            }
        } 
        else {
            if (expect_filename) {
                expect_filename = 0;
                ozhcom = 0;
            }
            else if (ozhcom) {
                ozhcom = 0;
            }
            else {
                return 1;
            }
        }
    }
    
    if (sum != 0 || ozhcom || expect_filename) {
        return 1;
    }
    
    return 0;
}