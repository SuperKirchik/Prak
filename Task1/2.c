#include <stdio.h>

int main() {
    char c = -31;
    if (c < 0) {
        printf("char is signed\n");
    } else {
        printf("char is unsigned\n");
    }
    return 0;
}