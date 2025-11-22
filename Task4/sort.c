#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LENGTH 256

char *lines[MAX_LINES];
int line_count = 0;

int compare_string(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

int compare_string_reverse(const void *a, const void *b) {
    return -strcmp(*(const char **)a, *(const char **)b);
}

int compare_string_case(const void *a, const void *b) {
    return strcasecmp(*(const char **)a, *(const char **)b);
}

int compare_numeric(const void *a, const void *b) {
    double num1 = atof(*(const char **)a);
    double num2 = atof(*(const char **)b);
    return (num1 > num2) - (num1 < num2);
}

void read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("sort");
        exit(1);
    }
    
    char buffer[MAX_LENGTH];
    while (fgets(buffer, sizeof(buffer), file) && line_count < MAX_LINES) {
        lines[line_count] = malloc(strlen(buffer) + 1);
        strcpy(lines[line_count], buffer);
        line_count++;
    }
    
    fclose(file);
}

int main(int argc, char *argv[]) {
    int r_flag = 0, f_flag = 0, n_flag = 0;
    char *filename = NULL;
    
    // Обработка аргументов
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strchr(argv[i], 'r')) r_flag = 1;
            if (strchr(argv[i], 'f')) f_flag = 1;
            if (strchr(argv[i], 'n')) n_flag = 1;
        } else {
            filename = argv[i];
        }
    }
    
    if (!filename) {
        fprintf(stderr, "Usage: %s [-rfn] filename\n", argv[0]);
        return 1;
    }
    
    read_file(filename);
    
    // Выбор функции сравнения
    int (*compare_func)(const void *, const void *);
    if (n_flag) {
        compare_func = compare_numeric;
    } else if (f_flag) {
        compare_func = compare_string_case;
    } else {
        compare_func = compare_string;
    }
    
    if (r_flag && !n_flag) {
        compare_func = compare_string_reverse;
    }
    
    qsort(lines, line_count, sizeof(char *), compare_func);
    
    // Вывод отсортированных строк
    for (int i = 0; i < line_count; i++) {
        printf("%s", lines[i]);
        free(lines[i]);
    }
    
    return 0;
}