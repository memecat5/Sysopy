// Linux only

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int main(int argc, char* argv[]){
    if(argc != 2){
        printf("Give exactly one argument!\n");
        return -1;
    }

    // No error handling, with wrong input it just returns 0
    int childrenCount = atoi(argv[1]);


    for(int i = 0; i < childrenCount; i++){
        int child_pid = fork();
        
        
        if(child_pid == 0){
            //Child code
            printf("PPID: %i, PID: %i\n", getppid(), getpid());
            return 0;
        }

        //Wait for the child to finnish
        waitpid(child_pid, NULL, 0);
    }

    printf("I'm parent - %i\n", childrenCount);

    return 0;
}
