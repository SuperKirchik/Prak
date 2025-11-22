#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int s_flag = 0;
    char *original = NULL;
    char *newlink = NULL;
    
    // Разбор аргументов
    if (argc < 3) {
        fprintf(stderr, "Usage: %s [-s] original new_link\n", argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "-s") == 0) {
        s_flag = 1;
        if (argc != 4) {
            fprintf(stderr, "Usage: %s [-s] original new_link\n", argv[0]);
            return 1;
        }
        original = argv[2];
        newlink = argv[3];
    } else {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s [-s] original new_link\n", argv[0]);
            return 1;
        }
        original = argv[1];
        newlink = argv[2];
    }
    
    // Создание ссылки
    if (s_flag) {
        if (symlink(original, newlink) == -1) {
            perror("ln");
            return 1;
        }
    } else {
        if (link(original, newlink) == -1) {
            perror("ln");
            return 1;
        }
    }
    
    return 0;
}