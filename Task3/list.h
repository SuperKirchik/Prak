#ifndef LIST_H
#define LIST_H

// Структура для списка строк
typedef struct {
    char **items;      // массив указателей на строки
    size_t size;       // текущее количество элементов
    size_t capacity;   // текущая вместимость
} StringList;

// Инициализация списка
void init_list(StringList *list);

// Добавление строки в конец списка
void add_string(StringList *list, const char *str);

// Сортировка списка (лексикографическая)
void sort_list(StringList *list);

// Вывод списка (сначала длина, затем каждый элемент)
void print_list(const StringList *list);

// Очистка памяти, занятой списком
void free_list(StringList *list);

#endif