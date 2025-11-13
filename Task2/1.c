#include <stdio.h>

int main() {
    double ep;
    scanf("%lf", &ep);
    
    double x;
    while (scanf("%lf", &x) == 1) {
        if (x < ep && x >= 0 || -x<ep && x < 0) {
            printf("%.10g\n", 0.0);
            continue;
        }
        
        double cur = 1.0;
        double next;
        
        while (1) {
            next = 0.5 * (cur + x / cur);
            if ((next - cur < ep && cur - next < ep) || 
                (cur - next < ep && next - cur < ep)) {
                break;
            }
            cur = next;
        }
        
        printf("%.10g\n", next);
    }
    
    return 0;
}