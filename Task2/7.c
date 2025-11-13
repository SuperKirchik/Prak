#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>

jmp_buf begin;
char curlex;
int s = 0;

void getlex(void);
int expr(void);
int term(void);
int factor(void);
int power(void);
void error(void);

int main() {
    int result;
    setjmp(begin);
    printf("==>");
    getlex();
    result = expr();
    if (curlex != '\n') error();
    printf("\n%d\n", result);
    printf("\n%d\n", s);
    return 0;
}

void getlex() {
    while ((curlex = getchar()) == ' ');
}

void error(void) {
    printf("\nОШИБКА!\n");
    while (getchar() != '\n');
    longjmp(begin, 1);
}

int expr() {
    int e = term();
    while (curlex == '+' || curlex == '-') {
        if (curlex == '+') {
            getlex();
            e += term();
        } else {
            getlex();
            e -= term();
        }
    }
    return e;
}

int term() {
    int t = factor();
    while (curlex == '*' || curlex == '/') {
        if (curlex == '*') {
            getlex();
            t *= factor();
        } else {
            getlex();
            int f = factor();
            if (f == 0) error();
            t /= f;
        }
    }
    return t;
}

int factor() {
    int f = power();
    while (curlex == '^') {
        getlex();
        int p = power();
        if (p < 0) error();
        int result = 1;
        for (int i = 0; i < p; i++) {
            result *= f;
        }
        f = result;
    }
    return f;
}

int power() {
    int p;
    switch (curlex) {
        case '0': case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9': 
            p = curlex - '0'; 
            break;
        case '(': 
            s += 2;
            getlex(); 
            p = expr();
            if (curlex != ')') error();
            break;
        default: 
            error();
    }
    getlex();
    return p;
}