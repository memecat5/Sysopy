#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

double f(double x){
    return 4.0 / (1.0 + pow(x, 2.0));
}

struct args{
    int n;
    double h;
    double slice;
    double *results;
    int *ready;
};

void calculations(struct args *args){
    int i = args->n;
    double h = args->h;

    for(int j = 0; j*h < args->slice; j++){
        args->results[i] += h * f(i * args->slice + j*h);
    }

    args->ready[i] = 1;

    return;
}

int check_ready(int ready[], int n){
    for(int i = 0; i<n; i++){
        if(ready[i] != 1){
            return 0;
        }
    }

    return 1;
}



int main(int argc, char* argv[]){
    if(argc != 3){
        printf("Give exactly two arguments!\n");
        return 1;
    }
    
    double h = atof(argv[1]);
    if(h > 1 || h < 0){
        printf("Invalid argument\n");
        return 1;
    }

    int n = atoi(argv[2]);

    double slice_for_thread;

    double result;

    // alloc maximal needes size for arrays
    double *results = malloc(sizeof(double) * n);
    int *ready = malloc(sizeof(int) * n);
    pthread_t *threads = malloc(sizeof(pthread_t) * n);
    struct args *arg_array = malloc(sizeof(struct args) * n);
    

    for(int threads_number = 1; threads_number <= n; threads_number++){
        printf("Liczba watkow: %d\n", threads_number);
        slice_for_thread = 1.0 / threads_number;
        result = 0;

        struct timeval start_time, end_time;

        gettimeofday(&start_time, NULL);

        for(int i = 0; i < threads_number; i++){
            results[i] = 0.0;
            ready[i] = 0;
        }

        for(int i = 0; i < threads_number; i++){
            arg_array[i] = (struct args){.n = i, .h = h, .slice = slice_for_thread, .results = results, .ready = ready};

            pthread_create(&threads[i], NULL, calculations, &arg_array[i]);
        }

        while(!check_ready(ready, threads_number)){}

        for(int i = 0; i < threads_number; i++){
            result += results[i];
        }

        gettimeofday(&end_time, NULL);
        
        double elapsed = (end_time.tv_sec - start_time.tv_sec) + 
                (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
        
        printf("Czas: %f s\n", elapsed);

        printf("Wynik: %f\n", result);

    }

    free(arg_array);
    free(results);
    free(ready);
    free(threads);

    return 0;
}
