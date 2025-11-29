#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tokenizer.h"

// Список специальных слов
const char *special_words[] = {"||", "&&", ">>", "<<", "|", "&", ";", ">", "<", "(", ")", NULL};

// Проверка, является ли строка специальным словом
int is_special_word(const char *str, size_t len) {
    for (int i = 0; special_words[i] != NULL; i++) {
        size_t special_len = strlen(special_words[i]);
        if (strncmp(str, special_words[i], len) == 0 && special_len == len) {
            return 1;
        }
    }
    return 0;
}

// Получение максимальной длины специального слова, начинающегося с данной позиции
size_t special_word_dlina(const char *str) {
    size_t max_len = 0;
    for (int i = 0; special_words[i] != NULL; i++) {
        size_t len = strlen(special_words[i]);
        if (strncmp(str, special_words[i], len) == 0 && len > max_len) {
            max_len = len;
        }
    }
    return max_len;
}

void del_string(const char *input, StringList *tokens) {
    size_t i = 0;
    size_t len = strlen(input);
    
    while (i < len) {
        // Пропускаем пробельные символы
        if (isspace(input[i])) {
            i++;
            continue;
        }
        
        // Проверяем, является ли текущая позиция началом специального слова
        size_t special_len = special_word_dlina(input + i);
        if (special_len > 0) {
            char *special_word = malloc(special_len + 1);
            strncpy(special_word, input + i, special_len);
            special_word[special_len] = '\0';
            add_string(tokens, special_word);
            free(special_word);
            i += special_len;
            continue;
        }
        
        // Обычное слово
        size_t start = i;
        while (i < len && !isspace(input[i]) && special_word_dlina(input + i) == 0) {
            i++;
        }
        
        if (i > start) {
            size_t word_len = i - start;
            char *word = malloc(word_len + 1);
            strncpy(word, input + start, word_len);
            word[word_len] = '\0';
            add_string(tokens, word);
            free(word);
        }
    }
}