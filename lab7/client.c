#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#define SERVER_QUEUE "/server_queue"
#define MAX_MSG_SIZE 1024
#define MAX_MSG_TEXT_SIZE 1016

typedef enum { INIT = 1, MESSAGE = 2 } msg_type;

typedef struct {
    msg_type type;
    int client_id;
    char text[MAX_MSG_TEXT_SIZE];
} message_t;
// sizeof(message_t) = 1024

char client_queue_name[MAX_MSG_TEXT_SIZE];
mqd_t client_q;
int client_id = -1;

void cleanup() {
    mq_close(client_q);
    mq_unlink(client_queue_name);
}

void handle_signal(int sig) {
    exit(0);
}

int main() {
    signal(SIGINT, handle_signal);
    atexit(cleanup);

    //queue name based on pid
    sprintf(client_queue_name, "/client_%d", getpid());

    // create client's queue
    struct mq_attr attr = { .mq_flags = 0, .mq_maxmsg = 10, .mq_msgsize = MAX_MSG_SIZE, .mq_curmsgs = 0 };
    client_q = mq_open(client_queue_name, O_CREAT | O_RDONLY, 0666, &attr);
    if (client_q == -1) {
        perror("mq_open client");
        exit(1);
    }

    // open server's queue
    mqd_t server_q = mq_open(SERVER_QUEUE, O_WRONLY);
    if (server_q == -1) {
        perror("mq_open server");
        exit(1);
    }

    // send INIT
    message_t msg = { .type = INIT };
    strncpy(msg.text, client_queue_name, sizeof(msg.text));
    mq_send(server_q, (char*)&msg, MAX_MSG_SIZE, 0);

    // receive client ID
    mq_receive(client_q, (char*)&msg, MAX_MSG_SIZE, NULL);
    client_id = msg.client_id;
    printf("Connected with client ID: %d\n", client_id);


    // start receiver process
    if(fork() == 0){
        message_t msg;
        while (1) {
            if (mq_receive(client_q, (char*)&msg, MAX_MSG_SIZE, NULL) > 0) {
                if (msg.type == MESSAGE) {
                    printf("Client %d says: %s", msg.client_id, msg.text);
                }
            }
        }
    } else{

        // main loop: read from stdin and send messages to server
        char buffer[MAX_MSG_TEXT_SIZE];
        while (fgets(buffer, sizeof(buffer), stdin)) {
            message_t out_msg = {
                .type = MESSAGE,
                .client_id = client_id
            };
            strncpy(out_msg.text, buffer, sizeof(out_msg.text));
            mq_send(server_q, (char*)&out_msg, MAX_MSG_SIZE, 0);
        }
    }
    return 0;
}
