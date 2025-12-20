#ifndef LIST_H
#define LIST_H

typedef struct {
    char **items;      // массив указателей на строки
    int size;       // текущее количество элементов
    int cur;   // текущая вместимость
} StringList;

typedef struct {
    char *buf;      
    int size;       
    int cur; 
} buffer;

int find_serror(StringList *list);
int pprov_spec(char c);
int prov_spec(char c);
void clearlist(StringList *list);
void nullst(StringList *list);
void termlist(StringList *list);
void nullbuf(buffer *buff);
void addsym(buffer *buff, char c);
void addword(buffer *buff, StringList *list);
void printlist(StringList *list);
int symset(char c);
void replace_variables(StringList *list);
int prov_raz(char *list);

#endif