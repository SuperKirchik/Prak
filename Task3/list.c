#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

#define DLINA 10

void init_list(StringList *list) {
    list->items = malloc(DLINA * sizeof(char *));
    list->size = 0;
    list->capacity = DLINA;
}

void add_string(StringList *list, const char *str) {
    if (list->size >= list->capacity) {
        list->capacity *= 2;
        list->items = realloc(list->items, list->capacity * sizeof(char *));
    }
    list->items[list->size] = malloc(strlen(str) + 1);
    strcpy(list->items[list->size], str);
    list->size++;
}

// Функция сравнения для сортировки
int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void sort_list(StringList *list) {
    // Простая сортировка пузырьком
    for (size_t i = 0; i < list->size - 1; i++) {
        for (size_t j = 0; j < list->size - i - 1; j++) {
            if (strcmp(list->items[j], list->items[j + 1]) > 0) {
                char *temp = list->items[j];
                list->items[j] = list->items[j + 1];
                list->items[j + 1] = temp;
            }
        }
    }
}

void print_list(const StringList *list) {
    printf("%zu\n", list->size);
    for (size_t i = 0; i < list->size; i++) {
        printf("%s\n", list->items[i]);
    }
}

void free_list(StringList *list) {
    for (size_t i = 0; i < list->size; i++) {
        free(list->items[i]);
    }
    free(list->items);
    list->size = 0;
    list->capacity = 0;
}