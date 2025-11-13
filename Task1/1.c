#include <stdio.h>

int main() {
    int short_size = sizeof(short);
    int int_size = sizeof(int);
    int long_size = sizeof(long);
    int float_size = sizeof(float);
    int double_size = sizeof(double);
    int long_double_size = sizeof(long double);
    
    printf("short: %d bytes\n", short_size);
    printf("int: %d bytes\n", int_size);
    printf("long: %d bytes\n", long_size);
    printf("float: %d bytes\n", float_size);
    printf("double: %d bytes\n", double_size);
    printf("long double: %d bytes\n", long_double_size);
    
    return 0;
}