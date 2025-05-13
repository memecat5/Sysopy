#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char* argv[]){
    if(argc != 3){
        printf("Give exactly 2 arguments\n");
        return 1;
    }
    int a = atoi(argv[1]);
    int b = atoi(argv[2]);
    
    if(a == b){
        printf("0\n");
        return 0;
    }
    if(a > b){
        int temp = b;
        b = a;
        a = temp;
    }

    char * fifo_path = "pipe";
    
    if(mkfifo(fifo_path, 0666) != 0){
        printf("Couldn't create pipe!\n");
        return 1;
    }
    
    int pipe = open("pipe", O_RDWR);

    if(pipe == 0){
        printf("Couldn't open pipe!\n");
        return 1;
    }


    if(write(pipe, &a, sizeof(int)) != sizeof(int)){
        printf("Error with writing to pipe!\n");
        return 1;
    }
    if(write(pipe, &b, sizeof(int)) != sizeof(int)){
        printf("Error with writing to pipe!\n");
        return 1;
    }

    FILE * calc = popen("./calculator", "w");
    pclose(calc);

    double result;
    
    if(read(pipe, &result, sizeof(double)) != sizeof(double)){
        printf("Error with reading from pipe!\n");
        return 1;
    }

    printf("Result: %f\n", result);

    close(pipe);

    unlink("pipe");

    return 0;
}