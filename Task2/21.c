#include <stdio.h> 

int main(){
    int result, x, koff;
    scanf("%d", &x);
    scanf("%d", &koff);
    result = koff;
    while (scanf("%d", &koff) == 1){
        result = result * x + koff;
    }
    printf("%d", result);
    return 0;
}