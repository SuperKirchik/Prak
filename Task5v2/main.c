#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <setjmp.h>
#include "lexer.h"
#include "tree.h"
#include "exec.h"

// Объявляем внешние переменные
extern jmp_buf start_buf;
int ex = 0;

int main() {
    signal(SIGINT, SIG_IGN);
    size_t buf_size = 64;
    char *input = malloc(buf_size);
    
    printf("My Shell (type 'exit' to quit)\n");
    
    while (1) {
        printf("myshell> ");
        fflush(stdout);
        
        int c;
        size_t pos = 0;
        
        while (1) {
            c = getchar();
            if (c == EOF || c == '\n') {
                if(c == EOF) ex = 1;
                input[pos] = '\0';
                break;
            }
            
            if (pos >= buf_size - 1) {
                buf_size *= 2;
                input = realloc(input, buf_size);
            }
            
            input[pos++] = c;
        }
        if(ex == 1){
            free(input);
            printf("Выход\n");
            break;
        }
        
        if (strcmp(input, "exit") == 0) {
            free(input);
            printf("Goodbye\n");
            break;
        }
        if (strlen(input) == 0) {
            continue;
        }
        
        token_list tokens;
        if (tokenize(input, &tokens) != 0) {
            printf("Tokenization error\n");
            continue;
        }
    
        expand_vars(&tokens);

        if (setjmp(start_buf) == 0) {
        tree *cmd_tree = build_tree(&tokens);
        
            if (cmd_tree) {

                int result = exec_tree(cmd_tree);
                
                if (shell_should_exit) {  
                    printf("Goodbye\n");
                    clear_list(&tokens);
                    free_tree(cmd_tree);
                    free(input);
                    exit(shell_exit_code);
                }
                
                free_tree(cmd_tree);
            } else {
                printf("Parse error\n");
            }
        } else {
            printf("Parse error\n");
        }
    
        clear_list(&tokens);
    }
    
    return 0;
}