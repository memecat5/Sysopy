#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

// Definitions of msg structs, types etc.
#include "common.c"

int sock_fd;

void on_sigint(int sig) {
    signal(SIGINT, SIG_IGN);
    Msg_to_server msg;
    msg.type = STOP;
    send(sock_fd, &msg, sizeof(msg), 0);
    close(sock_fd);
    exit(0);
}

void* receive_thread(void* arg) {
    Msg_to_client msg;
    while (1) {
        ssize_t bytes = recv(sock_fd, &msg, sizeof(msg), 0);
        if (bytes <= 0) {
            printf("Rozlaczono z serwerem.\n");
            exit(1);
        }

        if (msg.type == ALIVE) {
            // send ping back
            Msg_to_server alive_msg;
            alive_msg.type = ALIVE;
            send(sock_fd, &alive_msg, sizeof(alive_msg), 0);

        } //print message
        else if(msg.type == LIST){
            printf("[O %s]: %s\n", strtok(ctime(&msg.current_time), "\n"), msg.text);
        } else {
            printf("[Od %d o %s]: %s\n", msg.sender_id, strtok(ctime(&msg.current_time), "\n"), msg.text);
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Uzycie: %s <IP> <port>\n", argv[0]);
        return 1;
    }

    // disconnect on sigint
    signal(SIGINT, on_sigint);

    char* ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in server_addr;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return 1;
    }

    printf("Polaczono z serwerem\n");

    //thread for recieving messages from server
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_thread, NULL);

    char input[MAX_TEXT_LEN];
    // loop for creating messages
    while (1) {
        fgets(input, MAX_TEXT_LEN, stdin);
        input[strcspn(input, "\n")] = 0;

        Msg_to_server msg;
        if (strncmp(input, "LIST", 4) == 0) {
            msg.type = LIST;
            send(sock_fd, &msg, sizeof(msg), 0);

        } else if (strncmp(input, "2ALL ", 5) == 0) {
            msg.type = TO_ALL;
            strncpy(msg.text, input + 5, MAX_TEXT_LEN);
            send(sock_fd, &msg, sizeof(msg), 0);

        } else if (strncmp(input, "2ONE ", 5) == 0) {
            int target_id;
            if (sscanf(input + 5, "%d %[^\n]", &target_id, msg.text) >= 2) {
                msg.type = TO_ONE;
                msg.reciever_id = target_id;
                send(sock_fd, &msg, sizeof(msg), 0);
            } else {
                printf("Bledny format. Uzycie: 2ONE <id> <wiadomosc>\n");
            }

        } else if (strncmp(input, "STOP", 4) == 0) {
            msg.type = STOP;
            send(sock_fd, &msg, sizeof(msg), 0);
            break;

        } else {
            printf("Nieznana komenda. Dostepne: LIST, 2ALL, 2ONE, STOP\n");
        }
    }

    pthread_join(recv_thread, NULL);
    close(sock_fd);
    return 0;
}
