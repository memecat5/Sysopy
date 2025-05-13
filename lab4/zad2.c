// Linux only

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int global = 0;

int main(int argc, char* argv[]){
    if(argc != 2){
        printf("Give exactly one argument!\n");
        return -1;
    }
    
    printf("My name is %s\n", argv[0]);

    int local = 0;
    int child_pid;
    if(child_pid = fork() == 0){
        //Child code
        printf("Child process\n");
        global++;
        local++;

        printf("Child pid = %i, parent pid = %i\n", getpid(), getppid());
        printf("Child's local = %i, child's global = %i\n", local, global);

        return execl("/bin/ls", "./ls", argv[1], NULL);
    }
    else{
        int childExitCode;
        waitpid(child_pid, &childExitCode, 0);

        printf("Parent process\n");
        printf("Parent pid = %i, child pid = %i\n", getpid(), child_pid);
        printf("Child exit code: %i\n", childExitCode);
        printf("Parent's local = %i, parent's global = %i", local, global);

        return childExitCode;
    }
}