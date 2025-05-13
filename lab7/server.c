#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define SERVER_QUEUE "/server_queue"
#define MAX_CLIENTS 10
#define MAX_MSG_SIZE 1024
#define MAX_MSG_TEXT_SIZE 1016

typedef enum { INIT = 1, MESSAGE = 2 } msg_type;

typedef struct {
    msg_type type;      // init or message
    int client_id;
    char text[MAX_MSG_TEXT_SIZE];    //message text
} message_t;
// sizeof(message_t) = 1024

typedef struct {
    int client_id;
    mqd_t queue;
} client_t;

client_t clients[MAX_CLIENTS];
int client_count = 0;

void cleanup() {
    mq_unlink(SERVER_QUEUE);
}

void handle_sigint(int sig){
    exit(0);
}

int main() {
    // To unlink queue on sigint
    signal(SIGINT, handle_sigint);
    atexit(cleanup);


    struct mq_attr attr = { .mq_flags = 0, .mq_maxmsg = 10, .mq_msgsize = MAX_MSG_SIZE, .mq_curmsgs = 0 };
    mqd_t server_q = mq_open(SERVER_QUEUE, O_CREAT | O_RDONLY, 0666, &attr);
    if (server_q == -1) {
        perror("mq_open server");
        exit(1);
    }
    
    printf("Server is running...\n");

    message_t msg;
    while (1) {
        if (mq_receive(server_q, (char*)&msg, MAX_MSG_SIZE, NULL) == -1) {
            //error
            perror("mq_receive");
            continue;
        }

        //INIT from new client
        if (msg.type == INIT) {
            if (client_count >= MAX_CLIENTS) {
                printf("Too many clients, can't add more\n");
                continue;
            }

            // open client's message queue
            mqd_t client_q = mq_open(msg.text, O_WRONLY);
            if (client_q == -1) {
                perror("mq_open client");
                continue;
            }

            // save client's data
            int id = client_count;
            clients[client_count++] = (client_t){ id, client_q };
            
            // Sent response to INIT
            message_t resp = { .type = INIT, .client_id = id };
            mq_send(client_q, (char*)&resp, MAX_MSG_SIZE, 0);

            printf("Client connected: %s as ID %d\n", msg.text, id);

        } else if (msg.type == MESSAGE) {
            printf("Client %d: %s", msg.client_id, msg.text);

            // Send to all except sender
            for (int i = 0; i < client_count; i++) {
                if (clients[i].client_id != msg.client_id) {
                    mq_send(clients[i].queue, (char*)&msg, MAX_MSG_SIZE, 0);
                }
            }
        }
    }

    return 0;
}
