#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

void print_character(unsigned char c) {
    if (isprint(c) && c != '\\') {
        printf("  %c", c);
    } else {
        switch (c) {
            case '\n': printf(" \\n"); break;
            case '\t': printf(" \\t"); break;
            case '\r': printf(" \\r"); break;
            case '\0': printf(" \\0"); break;
            case '\\': printf(" \\\\"); break;
            default: printf(" %03o", c); break;
        }
    }
}

void od_file(const char *filename, int b_flag) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("od");
        exit(1);
    }
    
    unsigned char buffer[10];
    size_t offset = 0;
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, 10, file)) > 0) {
        printf("%06lo ", offset);
        
        if (b_flag) {
            // Восьмеричный вывод
            for (size_t i = 0; i < bytes_read; i++) {
                printf(" %03o", buffer[i]);
            }
            for (size_t i = bytes_read; i < 10; i++) {
                printf("    ");
            }
        } else {
            // Символьный вывод
            for (size_t i = 0; i < bytes_read; i++) {
                print_character(buffer[i]);
            }
        }
        printf("\n");
        
        offset += bytes_read;
    }
    
    fclose(file);
}

int main(int argc, char *argv[]) {
    int b_flag = 0;
    char *filename = NULL;
    
    // Обработка аргументов
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-b") == 0) {
                b_flag = 1;
            }
        } else {
            filename = argv[i];
        }
    }
    
    if (!filename) {
        fprintf(stderr, "Usage: %s [-b] filename\n", argv[0]);
        return 1;
    }
    
    od_file(filename, b_flag);
    return 0;
}