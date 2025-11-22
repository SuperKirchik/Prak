#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int n_flag = 0;
    int e_flag = 0;
    
    // Обработка флагов
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-n") == 0) {
                n_flag = 1;
            } else if (strcmp(argv[i], "-e") == 0) {
                e_flag = 1;
            }
        }
    }
    
    // Вывод аргументов
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') continue;
        
        if (e_flag) {
            // Простая обработка escape-последовательностей
            char *p = argv[i];
            while (*p) {
                if (*p == '\\' && *(p+1) == 'n') {
                    putchar('\n');
                    p += 2;
                } else if (*p == '\\' && *(p+1) == 't') {
                    putchar('\t');
                    p += 2;
                } else {
                    putchar(*p);
                    p++;
                }
            }
        } else {
            printf("%s", argv[i]);
        }
        
        if (i < argc - 1) {
            printf(" ");
        }
    }
    
    if (!n_flag) {
        printf("\n");
    }
    
    return 0;
}