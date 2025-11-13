#include <stdio.h>
#include <string.h>

double str2double(char str[]){
    double res = 0.0;
    int i = 0;
    while(str[i] != '.'){
        res *= 10;
        res += str[i] - '0';
        i++;
    }
    i = i+1;
    int k = 0;
    while(i < strlen(str)){
        res *= 10;
        res += str[i] - '0';
        i++;
        k++;
    }
    for(int j = 0; j < k; j++){
        res = res / 10;
    }
    return res;
}

int main(){
    char str[20];
    while(scanf("%s", str) == 1){
        printf("%f \n", str2double(str));
    }
    return 0;
}