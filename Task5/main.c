#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "list.h"
#include "tree.h"
#include "exec.h"

#define MAX_INPUT 1024

void print_prompt() {
    char cwd[1024];
    char hostname[256];
    
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strcpy(cwd, "?");
    }
    
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strcpy(hostname, "localhost");
    }
    
    printf("\033[1;32m%s@%s\033[0m:\033[1;34m%s\033[0m$ ", 
           getenv("USER"), hostname, cwd);
}

int main() {
    char input[MAX_INPUT];
    StringList lst;
    buffer buff;
    
    typedef enum {Start, Word, Greater, Greater2, Newline, Stop} vertex;
    
    setup_signal_handlers();
    printf("MyShell v1.0 (для выхода введите 'exit' или Ctrl+D)\n");
    
    while(1) {
        execute_background_processes();
        print_prompt();
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\nВыход из shell.\n");
            break;
        }
        
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0) {
            continue;
        }
        
        nullst(&lst);
        nullbuf(&buff);
        
        int i = 0;
        vertex V = Start;
        
        while (input[i] != '\0') {
            char c = input[i++];
            
            switch(V) {
                case Start:
                    if (c == ' ' || c == '\t') {
                    } else if (c == '#') {
                        while (input[i] != '\0') i++;
                    } else if (c == '\n' || c == '\0') {
                        termlist(&lst);
                        V = Newline;
                    } else {
                        nullbuf(&buff);
                        addsym(&buff, c);
                        V = (prov_spec(c)) ? Greater : Word;
                    }
                    break;
                    
                case Word:
                    if (symset(c) && c != '#') {
                        addsym(&buff, c);
                    } else {
                        V = Start;
                        addword(&buff, &lst);
                        i--;
                    }
                    break;
                    
                case Greater:
                    if (pprov_spec(c)) {
                        addsym(&buff, c);
                        V = Greater2;
                    } else {
                        V = Start;
                        addword(&buff, &lst);
                        i--;
                    }
                    break;
                    
                case Greater2:
                    V = Start;
                    addword(&buff, &lst);
                    i--;
                    break;
                    
                case Newline:
                    termlist(&lst);
                    break;
                    
                default:
                    break;
            }
        }
        
        if (V == Word || V == Greater || V == Greater2) {
            addword(&buff, &lst);
        }
        termlist(&lst);
        
        if (find_serror(&lst) != 0) {
            printf("Ошибка синтаксиса\n");
            clearlist(&lst);
            continue;
        }
        
        replace_variables(&lst);
        
        if (lst.items == NULL || lst.items[0] == NULL) {
            clearlist(&lst);
            continue;
        }
        
        tree cmd_tree = build_tree(&lst);
        if (cmd_tree == NULL) {
            printf("Ошибка построения дерева команд\n");
            clearlist(&lst);
            continue;
        }
        
        execute_tree(cmd_tree);
        
        clear_tree(cmd_tree);
        clearlist(&lst);
    }
    
    return 0;
}