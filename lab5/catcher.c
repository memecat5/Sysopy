#define _POSIX_C_SOURCE 199309L

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

int catcher_mode = 0;
int mode_changes = 0;

void set_mode(int mode){
    if(mode < 1 || mode > 5){
        // default for invalid request
        catcher_mode = 5;
    }
    else{
        catcher_mode = mode;
    }
    mode_changes++;
}

void SIGUSR1_handler(int sig, siginfo_t *siginfo, void *ucontext){
    int senders_pid = siginfo->si_pid;
    int mode = siginfo->si_value.sival_int;

    //set working mode
    set_mode(mode);

    //confirm recieved signal to sender
    kill(senders_pid, SIGUSR1);
}

void SIGINT_handler(int sig){
    printf("Ctrl+C pressed\n");

    // reapply signal handling
    signal(SIGINT, SIGINT_handler);
}


int main(){

    printf("Catcher's PID: %i\n", getpid());

    struct sigaction act;
    act.sa_sigaction = SIGUSR1_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);

    int number;

    while(1){
        switch (catcher_mode){
            // requested action completed,
            // waiting for furher instructions

            // this also the initial state
            case 0:
                pause();
            break;

            //inform about recieved requests
            case 1:
                printf("Recieved requests to change mode: %i\n", mode_changes);
                catcher_mode = 0;
            break;

            // print numbers
            case 2:
                number = 0;
                while(catcher_mode == 2){
                    printf("%i\n", number++);
                    sleep(1);
                }
            break;
            
            // ignore ctrl + c
            case 3:
                signal(SIGINT, SIG_IGN);
                catcher_mode = 0;
            break;

            //inform about ctrl + c
            case 4:
                signal(SIGINT, SIGINT_handler);
                catcher_mode = 0;
            break;

            // finnish
            case 5:
                return 0;
            break;

            // invalid argument
            default:
                return -1;
            break;
        }
    }
}