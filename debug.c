#include "cave.h"

//So I can place these all over my code, when debugging
int print_all = 1;

void print(char *s){
    if (print_all != 0) {
        printf("%s\n",s);
    }
}

void pint(int i){
    if (print_all != 0) {
        printf("%d\n",i);
    }
}


