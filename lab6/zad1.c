#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>

double f(double x){
    return 4.0 / (1.0 + pow(x, 2.0));
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

    double result;
    double slice_for_process;

    // seperate pipe for each child
    int (*pipes)[2] = malloc(sizeof(*pipes) * n);
    for(int i=0; i<n; i++){
        if(pipe(pipes[i]) == -1){
            printf("Pipe failed :(\n");
            return 1;
        }
    }

    for(int process_no = 1; process_no <= n; process_no++){
        printf("K = %i\n", process_no);
        
        slice_for_process = 1.0 / process_no;

        struct timeval start_time, end_time;

        gettimeofday(&start_time, NULL);

        for(int i = 0; i < process_no; i++){
            if(fork() == 0){
                //might close all of them but it seems like a waste of time
                for(int i=0; i<n; i++){
                    close(pipes[i][0]);
                }
                double val = 0.0;
                int j = 0;

                // simplified while i*slice_for_process + j*h < (i+1)*slice_for_process
                // in other words, while process is calculating within it's range
                for(int j = 0; j*h < slice_for_process; j++){
                    // leftpoint method
                    val += h * f(i*slice_for_process + j*h);
                }

                if(write(pipes[i][1], &val, sizeof(double)) != sizeof(double)){
                    printf("Write error\n");
                    return -1;
                }

                return 0;
            }
        }

        result = 0.0;
        double temp;
        for(int i = 0; i < process_no; i++){
            if(read(pipes[i][0], &temp, sizeof(double)) != sizeof(double)){
                printf("Read error\n");
                return -1;
            }
            result += temp;
        }

        gettimeofday(&end_time, NULL);
        
        double elapsed = (end_time.tv_sec - start_time.tv_sec) + 
                (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
        
        printf("Time: %f s\n", elapsed);

        printf("Result: %f\n", result);
    }

    for(int i=0; i<n; i++){
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    free(pipes);
}
