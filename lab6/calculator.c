#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

#define SLICES 1000

double f(double x){
    return 4.0 / (1.0 + pow(x, 2.0));
}

int main(){

    int pipe_fd = open("pipe", O_RDWR);
    if(pipe_fd == 0){
        printf("Couldn't open pipe!\n");
        return 1;
    }

    int a, b;
    if(read(pipe_fd, &a, sizeof(int)) != sizeof(int)){
        printf("Error with reading from pipe!\n");
        return 1;
    }
    if(read(pipe_fd, &b, sizeof(int)) != sizeof(int)){
        printf("Error with reading from pipe!\n");
        return 1;
    }

    double h = (double)(b - a) / (double)SLICES;
    double result = 0.0;
    
    for(int i = 0; i < SLICES; i++){
        //rightpoint method
        result += f(a + i*h) * h;
    }

    if(write(pipe_fd, &result, sizeof(double)) != sizeof(double)){
        printf("Error with writing to pipe!\n");
        return 1;
    }

    close(pipe_fd);

    return 0;
}