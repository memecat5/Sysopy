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

Client clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// to close it on ctrl+c
int server_fd;

void list_active_clients(int sender_id){
    char text[MAX_TEXT_LEN] = "Aktywni klienci:";
    int pos = (int)strlen(text);
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].active){
            pos += sprintf(text + pos, " %d,", i);
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    sprintf(text + pos, "\n");
    Msg_to_client msg_out = {.type = LIST, .current_time = time(NULL)};
    strcpy(msg_out.text, text);
    send(clients[sender_id].socket_fd, &msg_out, sizeof(msg_out), 0);
}

void broadcast_message(Msg_to_server* msg_in, int sender_id){
    Msg_to_client msg_out = {.type = TO_ALL, .sender_id = sender_id, .current_time = time(NULL)};
    strcpy(&msg_out.text, (*msg_in).text);

    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].active && clients[i].id != sender_id){
            send(clients[i].socket_fd, &msg_out, sizeof(msg_out), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Thread that recieves messages from one clients and responses
// accordingly (sends requested messages and notes alive ping)
void* handle_client(int* arg){
    Client *client = &clients[*arg];
    Msg_to_server msg_in;
    int bytes;

    while(bytes = recv(client->socket_fd, (char*)&msg_in, sizeof(msg_in), 0) > 0){
        if(msg_in.type == LIST){
            list_active_clients(client->id);

        } else if(msg_in.type == TO_ALL){
            broadcast_message(&msg_in, client->id);

        } else if(msg_in.type == TO_ONE){
            Msg_to_client msg_out = {.type = TO_ONE, .sender_id = client->id, .current_time = time(NULL)};
            strcpy(&msg_out.text, &msg_in.text);
            send(clients[msg_in.reciever_id].socket_fd, &msg_out, sizeof(msg_out), 0);

        } else if(msg_in.type == STOP){
            break;

        } else if(msg_in.type == ALIVE){
            //client signals that it's alive (asked by other thread)
            pthread_mutex_lock(&clients_mutex);
            client->awaiting_ping = 0;
            pthread_mutex_unlock(&clients_mutex);
        }
    }

    printf("Odlaczam klienta %d\n", client->id);
    pthread_mutex_lock(&clients_mutex);

    // if not, it means that alive_checker killed it
    if(client->active){
        close(client->socket_fd);
        client->active = 0;
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Thread that will continously ping clients every minute if they were inactive for >30 seconds
// If they don't respond within 1 minute (it has 1 minute cycles) it will set this client inactive
// and disconnet it.
// If the client sends a response, it's noted by handle_client thread in awaiting_ping flag
void* alive_pings(){
    while(1){
        sleep(60);
        time_t now = time(NULL);
        pthread_mutex_lock(&clients_mutex);
        for(int i = 0; i < MAX_CLIENTS; i++){
            //ping was sent in previous iteration and still no response
            if(clients[i].awaiting_ping){
                clients[i].awaiting_ping = 0;
                clients[i].active = 0;
                close(clients[i].socket_fd);
                //seems dangerous
                //pthread_cancel(clients[i].thread);
            }else{
                // ping client
                if(clients[i].active && (now - clients[i].last_active) > 30){
                    Msg_to_client msg_out = {.type = ALIVE};
                    send(clients[i].socket_fd, &msg_out, sizeof(msg_out), 0);
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
}

void on_sigint(int sig){
    signal(SIGINT, SIG_IGN);
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].active){
            close(clients[i].socket_fd);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    close(server_fd);
    exit(0);
}

int main(int argc, char* argv[]){
    signal(SIGINT, on_sigint);
    
    if(argc != 2){
        printf("Give exactly one argument - port for server!\n");
        return 1;
    }

    int lient_fd;
    struct sockaddr_in server_addr, client_addr;

    socklen_t addr_len = sizeof(server_addr);

    int port = atoi(argv[1]);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0){
        perror("Socket failure\n");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("Bind");
        return 1;
    }

    if(listen(server_fd, 5) < 0){
        perror("listen");
        return 1;
    }

    printf("Serwer nasluchuje na porcie %d\n", port);

    while(1){
        // connect new client
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if(client_fd < 0){
            perror("accept client\n");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);

        // if there is an empty space for new client it will create
        // a new thread to handle that client, if not it prints
        // error message and disconnects this new client
        int slot_found = 0;
        for(int i = 0; i < MAX_CLIENTS; i++){
            if(!clients[i].active){
                clients[i].socket_fd = client_fd;
                clients[i].addr = client_addr;
                clients[i].active = 1;
                clients[i].id = i;

                if(pthread_create(&clients[i].thread, NULL, handle_client, &clients[i].id) != 0){
                    perror("create client handler thread");
                    clients[i].active = 0;
                    close(client_fd);
                }
                printf("Polaczono klienta %d\n", i);
                slot_found = 1;
                break;
            }
        }
        // all slots are full, disconnect this client
        if(!slot_found){
            printf("Osiagnieto maksymalna liczbe klientow (%d)\n", MAX_CLIENTS);
            close(client_fd);
        }
        pthread_mutex_unlock(&clients_mutex);

    }

    return 0;
}