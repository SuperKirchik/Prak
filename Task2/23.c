#include <stdio.h>

int main() {
    int n;
    double x, koff;
    double result = 0, prois = 0, integral = 0;
    int i;
    
    scanf("%d", &n);
    scanf("%lf", &x);
    
    scanf("%lf", &koff);
    result = koff;
    prois = n * koff;
    integral = koff / (n + 1);
    
    for (i = n - 1; i >= 0; i--) {
        scanf("%lf", &koff);
        result = result * x + koff;
        if (i > 0) {
            prois = prois * x + i * koff;
        }
        integral = integral * x + koff / (i + 1);
    }
    integral = integral * x;
    
    printf("%lf\n", result);
    printf("%lf\n", prois);
    printf("%lf\n", integral);
    
    return 0;
}