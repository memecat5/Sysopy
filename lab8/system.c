#define _C_POSIX_SOURCE 200809L
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define SEM_QUEUE_FULL "sem_full"
#define SEM_QUEUE_EMPTY "sem_empty"
#define MEM_NAME "memory"
#define MSG_SIZE sizeof(char) * 11  //11 for \0
#define MSG_LEN 10

// Read msg to print, wait if necessary
void queue_read(sem_t* full, sem_t* empty, char* queue, char* buff){
    sem_wait(full);
    strcpy(buff, queue);
    sem_post(empty);
}

// Write msg to printing, wait if necessary
void queue_write(sem_t* full, sem_t* empty, char* queue, char* buff){
    sem_wait(empty);
    strcpy(queue, buff);
    sem_post(full);
}

void printer_process(){
    sem_t* queue_empty = sem_open(SEM_QUEUE_EMPTY, O_RDWR);
    sem_t* queue_full = sem_open(SEM_QUEUE_FULL, O_RDWR);

    int shm_fd = shm_open(MEM_NAME, O_RDWR, 0666);
    char* print_queue = (char*)mmap(NULL, MSG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    char buff[MSG_LEN+1];

    // printing loop
    while(1){
        queue_read(queue_full, queue_empty, print_queue, buff);
        for(int i = 0; i < MSG_LEN; i++){
            printf("Printer %d: %c\n", getpid(), buff[i]);
            sleep(1);
        }
        printf("Printer %d finished printing %s\n\n", getpid(), buff);
    }
}

void user_process(){
    srand(getpid());
    sem_t* queue_empty = sem_open(SEM_QUEUE_EMPTY, O_RDWR);
    sem_t* queue_full = sem_open(SEM_QUEUE_FULL, O_RDWR);

    int shm_fd = shm_open(MEM_NAME, O_RDWR, 0666);
    char* print_queue = (char*)mmap(NULL, MSG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    char msg[MSG_LEN + 1];

    // user loop
    while(1){
        // create random message
        for(int i = 0; i < MSG_LEN; i++){
            msg[i] = 'a' + rand() % 26;
        }
        msg[MSG_LEN] = '\0';

        // to not get confused
        printf("User %d writes: %s\n", getpid(), msg);
        
        // send message to printers
        queue_write(queue_full, queue_empty, print_queue, msg);

        sleep(rand() % 20 + 10);
    }

}

void on_sigint(int sig){
    shm_unlink(MEM_NAME);
    sem_unlink(SEM_QUEUE_EMPTY);
    sem_unlink(SEM_QUEUE_FULL);
    exit(0);
}


int main(int argc, char* argv[]){
    int m_printers;
    int n_users;
    // parameters: users printers
    if(argc == 1){
        //default
        n_users = 5;
        m_printers = 2;
    } else if(argc == 3){
        n_users = atoi(argv[1]);
        m_printers = atoi(argv[2]);
    } else{
        printf("Wrong character count!\n");
        return 1;
    }

    if(n_users <= 0 || m_printers <= 0){
        printf("Invalid arguments!\n");
        return 1;
    }

    signal(SIGINT, on_sigint);

    // Queue is just a space for one message, those semaphores inform
    // whether it's empty or full
    sem_t* queue_empty = sem_open(SEM_QUEUE_EMPTY, O_CREAT, 0666, 1);
    sem_t* queue_full = sem_open(SEM_QUEUE_FULL, O_CREAT, 0666, 0);

    int shm_fd = shm_open(MEM_NAME, O_CREAT | O_RDWR, 0666);

    if(ftruncate(shm_fd,  MSG_SIZE) != 0){
        perror("ftruncate went wrong\n");
        exit(1);
    }

    char* print_queue = (char*)mmap(NULL, MSG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);


    //create printer processes
    for(int i = 0; i < m_printers; i++){
        if(fork() == 0){
            printer_process();
            exit(0);
        }
    }

    //create user processes
    for(int i = 0; i < n_users; i++){
        if(fork() == 0){
            user_process();
            exit(0);
        }
    }


    pause();
}
