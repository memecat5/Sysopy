#define _POSIX_C_SOURCE 199309L

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

void handler(int sig){
    printf("Recieved confirmation\n");
}

int main(int argc, char* argv[]){
    if(argc != 3){
        printf("Wrong argument count!\n");
        return -1;
    }
    int catchers_pid = atoi(argv[1]);
    int catchers_workmode = atoi(argv[2]);

    if(catchers_pid == 0){
        printf("Wrong pid!\n");
        return -1;
    }

    union sigval val;
    val.sival_int = catchers_workmode;

    // send request to catcher
    sigqueue(catchers_pid, SIGUSR1, val);

    // set signal handling to not kill the process
    signal(SIGUSR1, handler);

    // wait for catcher's confirmation
    pause();

    return 0;
}