#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "tokenizer.h"

#define BUFFER_SIZE 1024

int main() {
    char buffer[BUFFER_SIZE];
    StringList tokens;
    init_list(&tokens);
    
    // Чтение до конца файла
    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        // Удаляем символ новой строки
        buffer[strcspn(buffer, "\n")] = 0;
        
        //Делим строку
        del_string(buffer, &tokens);
        
        // Выводим исходный порядок
        printf("Original order:\n");
        print_list(&tokens);
        
        // Сортируем и выводим
        printf("Sorted order:\n");
        sort_list(&tokens);
        print_list(&tokens);
        
        // Очищаем список для следующей строки
        free_list(&tokens);
        init_list(&tokens);
    }
    
    free_list(&tokens);
    return 0;
}