#include <stdio.h>
#include <dlfcn.h>
#include "lib_collatz.h"

#define MAX_ITERATIONS 100

int main(){
    #ifdef DLL
    //open library and get handler
    void *handle = dlopen("./lib_collatz.so", RTLD_LAZY);

    //dlsym takes library handler and function name, returns function pointer
    int (*test_collatz_convergence)(int, int, int*) = dlsym(handle, "test_collatz_convergence");
    #endif


    int test[] = {15, 3, 1024, 2137, 27};

    for(int i = 0; i < 5; i++){

        int steps[MAX_ITERATIONS] = {};

        int result = test_collatz_convergence(test[i], MAX_ITERATIONS, steps);
        
        printf("Testowana liczba: %i\n", test[i]);

        if(result != 0)
            for(int j = 0; j < result; j++){
                printf("%i\n", steps[j]);
            }
        else printf("Nie udalo sie osiagnac 1 w limicie %i\n", MAX_ITERATIONS);
    }

    #ifdef DLL
    //close library
    dlclose(handle);
    #endif
}