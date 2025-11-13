#include <stdio.h> 

int main(){
    int result, x, koff;
    int prois = 0;
    scanf("%d", &x);
    scanf("%d", &koff);
    result = koff;
    while (scanf("%d", &koff) == 1){
        prois = prois * x + result;
        result = result * x + koff;
    }
    printf("%d\n", result);
    printf("%d", prois);
    return 0;
}