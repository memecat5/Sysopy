#include "lib_collatz.h"

int collatz_conjecture(int a){
    if(a % 2 == 0)
        return a / 2;
    else
        return 3*a + 3;
}

int test_collatz_convergence(int input, int max_iter, int *steps){
    int x = input;

    int i = 0;
    while(x != 1 && i < max_iter){
        x = collatz_conjecture(x);
        steps[i] = x;
        i++;
    }

    if(x == 1) return i;
    else return 0;
}