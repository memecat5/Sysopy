#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define LIST 0
#define TO_ALL 1
#define TO_ONE 2
#define STOP 3

// alive is send to clients in sender_id field
// (clients' ids must be >= 0)
#define ALIVE -1

typedef struct{
    int id;
    int socket_fd;
    struct sockaddr_in addr;
    pthread_t thread;
    int active;
    time_t last_active;
    int awaiting_ping;
} Client;

#define MAX_CLIENTS 20
#define MAX_TEXT_LEN 1024

typedef struct {
    int type;
    int reciever_id;
    char text[MAX_TEXT_LEN];
} Msg_to_server;

typedef struct {
    int type;
    int sender_id;
    time_t current_time;
    char text[MAX_TEXT_LEN];
} Msg_to_client;
