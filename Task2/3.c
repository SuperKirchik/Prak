#include <stdio.h>

long long fib_iterative(int i,int *k) {
    if (i == 0) return 0;
    if (i == 1) return 1;
    
    long long a = 0, b = 1, c;
    for (int n = 2; n <= i; n++) {
        (*k) += 1;
        c = a + b;
        a = b;
        b = c;
    }
    return b;
}

long long fib_recursive(int i,int *k) {
    (*k)+=1;
    if (i == 0) return 0;
    if (i == 1) return 1;
    return fib_recursive(i - 2, k) + fib_recursive(i - 1, k);
}

int main() {
    int i, k_rec, k_iter;
    k_iter = 1;
    k_rec = 1;
    while (scanf("%d", &i) == 1) {
        long long result_iter = fib_iterative(i, &k_iter);
        printf("%lld\n", result_iter);
        printf("%d\n", k_iter);

        long long result_rec = fib_recursive(i, &k_rec);
        printf("%lld\n", result_rec);
        printf("%d\n", k_rec);
        printf("\n");
    }
    
    return 0;
}
