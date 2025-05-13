#define _POSIX_C_SOURCE 2

#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void handler(int sig){
    printf("Recieved SIGUSR1 signal\n");
}

int main(int argc, char* argv[]){
    if(argc != 2){
        printf("Wrong argument count!\n");
        return -1;
    }

    if(strcmp(argv[1], "none") == 0){
        //default
        signal(SIGUSR1, SIG_DFL);
    }else if(strcmp(argv[1], "ignore") == 0){
        signal(SIGUSR1, SIG_IGN);

    }else if(strcmp(argv[1], "handler") == 0){
        signal(SIGUSR1, handler);
        
    }else if(strcmp(argv[1], "mask") == 0){
        sigset_t old;
        sigset_t new;
        sigemptyset(&new);
        sigaddset(&new, SIGUSR1);
        if(sigprocmask(SIG_SETMASK, &new, &old) < 0){
            printf("Couldn't mask process\n");
        }

    }else{
        printf("Wrong argument!\n");
        return -1;
    }

    raise(SIGUSR1);
}