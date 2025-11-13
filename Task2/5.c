#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct WordNode {
    char* word;
    struct WordNode* next;
};

struct WordNode* create_node(const char* word) {
    struct WordNode* new_node = (struct WordNode*)malloc(sizeof(struct WordNode));
    if (new_node == NULL) return NULL;
    
    new_node->word = (char*)malloc(strlen(word) + 1);
    if (new_node->word == NULL) {
        free(new_node);
        return NULL;
    }
    strcpy(new_node->word, word);
    new_node->next = NULL;
    return new_node;
}

void add_to_end(struct WordNode** head, const char* word) {
    struct WordNode* new_node = create_node(word);
    if (new_node == NULL) return;
    
    if (*head == NULL) {
        *head = new_node;
        return;
    }
    
    struct WordNode* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
}

char* get_last_word(struct WordNode* head) {
    if (head == NULL) return NULL;
    
    struct WordNode* current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    return current->word;
}

void remove_dupl(struct WordNode** head) {
    if (*head == NULL) return;
    
    char* last_word = get_last_word(*head);
    if (last_word == NULL) return;
    
    struct WordNode* current = *head;
    struct WordNode* prev = NULL;
    
    while (current != NULL && current->next != NULL) {
        if (strcmp(current->word, last_word) == 0) {
            if (prev == NULL) {
                *head = current->next;
            } else {
                prev->next = current->next;
            }
            
            struct WordNode* to_delete = current;
            current = current->next;
            free(to_delete->word);
            free(to_delete);
        } else {
            prev = current;
            current = current->next;
        }
    }
}

void print_list(struct WordNode* head) {
    struct WordNode* current = head;
    int first = 1;
    
    while (current != NULL) {
        if (!first) {
            printf(" ");
        }
        printf("%s", current->word);
        first = 0;
        current = current->next;
    }
    printf("\n");
    current = head;
    int minim = 0;
    int maxim = 0;
    while (current != NULL) {
        printf("%lu' '", strlen(current->word));
        current = current->next;
    }
    printf("\n");
    current = head;
    if(current != NULL){
        minim = strlen(current->word);
        while(current->next != NULL){
            current = current->next;
        }
    }
    maxim = strlen(current->word);
    int s = (minim + maxim)/2;
    current = head;
    while (current != NULL) {
        if(strlen(current->word) != s){
            printf("%s ", current->word);
        }
        current = current->next;
    }
    printf("\n");
}




void sort(struct WordNode** head) {
    int k = 0;
    if (*head == NULL || (*head)->next == NULL) return;
    int swapped;
    do {
        swapped = 0;
        struct WordNode* current = *head;
        struct WordNode* prev = NULL;

        while (current != NULL && current->next != NULL) {
            struct WordNode* nextnode = current->next;
            if (strlen(current->word) > strlen(nextnode->word)) {
                if (prev == NULL) {
                    *head = nextnode;
                } else {
                    prev->next = nextnode;
                }
                current->next = nextnode->next;
                nextnode->next = current;
                swapped = 1;
                k++;
            }
            prev = current;
            current = current->next;
        }
    } while (swapped);
    printf("%d", k);
}

void free_list(struct WordNode* head) {
    struct WordNode* current = head;
    while (current != NULL) {
        struct WordNode* next = current->next;
        free(current->word);
        free(current);
        current = next;
    }
}

int main() {
    struct WordNode* word_list = NULL;
    char word[100];
    
    while (scanf("%99s", word) != EOF) {
        add_to_end(&word_list, word);
    }
    
    if (word_list != NULL) {
        remove_dupl(&word_list);
        sort(&word_list);
        print_list(word_list);
        free_list(word_list);
    } else {
        printf("\n");
    }
    
    return 0;
}