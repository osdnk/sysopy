#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <mqueue.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

#include "communication.h"

#define FAILURE_EXIT(format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(-1); }

void handle_public_queue(struct Message *msg);
void do_login(struct Message *msg);
void do_mirror(struct Message *msg);
void do_calc(struct Message *msg);
void do_time(struct Message *msg);
void do_end(struct Message *msg);
void do_quit(struct Message *msg);
int find_queue_id(pid_t sender_pid);
int prepare_msg(struct Message *msg);
char* convert_time(const time_t *mtime);
void close_queue();
void int_handler(int);

int active = 1;
int clients_data[MAX_CLIENTS][2];
int client_count = 0;
mqd_t queue_descriptor = -1;

// MAIN ////////////////////////////////////////////////////////////////////////

int main() {
    if (atexit(close_queue) == -1)
        FAILURE_EXIT("server: registering server's atexit failed\n");
    if (signal(SIGINT, int_handler) == SIG_ERR)
        FAILURE_EXIT("server: registering INT failed\n");

    struct mq_attr current_state;
    struct mq_attr posix_attr;
    posix_attr.mq_maxmsg = MAX_MESSAGE_QUEUE_SIZE;
    posix_attr.mq_msgsize = MESSAGE_SIZE;

    queue_descriptor = mq_open(server_path, O_RDONLY | O_CREAT | O_EXCL, 0666, &posix_attr);

    if (queue_descriptor == -1)
        FAILURE_EXIT("server: creation of public queue failed\n");

    Message buffer;
    while(1) {
        if(active == 0) {
            if (mq_getattr(queue_descriptor, &current_state) == -1)
                FAILURE_EXIT("server: couldnt read public queue parameters\n");
            if (current_state.mq_curmsgs == 0) exit(0);
        }

        if (mq_receive(queue_descriptor,(char*) &buffer, MESSAGE_SIZE, NULL) == -1)
            FAILURE_EXIT("server: receiving message by server failed\n");
        handle_public_queue(&buffer);
    }
}

void handle_public_queue(struct Message *msg) {
    if (msg == NULL) return;
    switch(msg->mtype) {
        case LOGIN:
            do_login(msg);
            break;
        case MIRROR:
            do_mirror(msg);
            break;
        case CALC:
            do_calc(msg);
            break;
        case TIME:
            do_time(msg);
            break;
        case END:
            do_end(msg);
            break;
        case QUIT:
            do_quit(msg);
            break;
        default:
            break;
    }
}

// HANDLERS ////////////////////////////////////////////////////////////////////

void do_login(struct Message *msg) {
    int clientPID = msg->sender_pid;
    char clientPath[15];
    sprintf(clientPath, "/%d", clientPID);

    int client_queue_id = mq_open(clientPath, O_WRONLY);
    if (client_queue_id == -1) FAILURE_EXIT("server: reading client_queue_id failed\n");

    msg->mtype = INIT;
    msg->sender_pid = getpid();

    if (client_count > MAX_CLIENTS - 1) {
        printf("server: maximum amount of clients reached\n");
        sprintf(msg->message_text, "%d", -1);
        if (mq_send(client_queue_id, (char*) msg, MESSAGE_SIZE, 1) == -1)
            FAILURE_EXIT("server: login response failed\n");
        if (mq_close(client_queue_id) == -1)
            FAILURE_EXIT("server: closing client's queue failed\n");
    } else {
        clients_data[client_count][0] = clientPID;
        clients_data[client_count++][1] = client_queue_id;
        sprintf(msg->message_text, "%d", client_count-1);
        if (mq_send(client_queue_id, (char*) msg, MESSAGE_SIZE, 1) == -1)
            FAILURE_EXIT("server: login response failed\n");
    }
}

void do_mirror(struct Message *msg) {
    int client_queue_id = prepare_msg(msg);
    if (client_queue_id == -1) return;

    int msgLen = (int) strlen(msg->message_text);
    if (msg->message_text[msgLen-1] == '\n') msgLen--;

    int i; for (i = 0; i < msgLen / 2; ++i) {
        char buff = msg->message_text[i];
        msg->message_text[i] = msg->message_text[msgLen - i - 1];
        msg->message_text[msgLen - i - 1] = buff;
    }

    if (mq_send(client_queue_id, (char*) msg, MESSAGE_SIZE, 1) == -1)
        FAILURE_EXIT("server: MIRROR response failed\n");
}

void do_calc(struct Message *msg) {
    int client_queue_id = prepare_msg(msg);
    if (client_queue_id == -1) return;

    char cmd[4108];
    sprintf(cmd, "echo '%s' | bc", msg->message_text);
    FILE* calc = popen(cmd, "r");
    fgets(msg->message_text, MAX_CONT_SIZE, calc);
    pclose(calc);

    if (mq_send(client_queue_id, (char*) msg, MESSAGE_SIZE, 1) == -1)
        FAILURE_EXIT("server: CALC response failed\n");
}

void do_time(struct Message *msg) {
    int client_queue_id = prepare_msg(msg);
    if (client_queue_id == -1) return;

    time_t timer;
    time(&timer);
    char* timeStr = convert_time(&timer);

    sprintf(msg->message_text, "%s", timeStr);
    free(timeStr);

    if (mq_send(client_queue_id, (char*) msg, MESSAGE_SIZE, 1) == -1)
        FAILURE_EXIT("server: TIME response failed");
}

void do_end(struct Message *_) { active = 0; }

void do_quit(struct Message *msg) {
    int i; for (i = 0; i<client_count; ++i) {
        if(clients_data[i][0] == msg->sender_pid) break;
    }
    if(i == client_count) {
        printf("server: client not found\n");
        return;
    }
    if (mq_close(clients_data[i][1]) == -1)
        FAILURE_EXIT("server: closing clients queue in QUIT response failed\n");
    for (; i + 1 < client_count; ++i) {
        clients_data[i][0] = clients_data[i + 1][0];
        clients_data[i][1] = clients_data[i + 1][1];
    }
    client_count--;
    printf("server: cleared data of removed client\n");
}

// HELPERS /////////////////////////////////////////////////////////////////////

int prepare_msg(struct Message *msg) {
    int client_queue_id  = find_queue_id(msg->sender_pid);
    if (client_queue_id == -1) {
        printf("server: client not found\n");
        return -1;
    }

    msg->mtype = msg->sender_pid;
    msg->sender_pid = getpid();

    return client_queue_id;
}

int find_queue_id(pid_t sender_pid) {
    int i; for (i = 0; i < client_count; ++i)
        if (clients_data[i][0] == sender_pid)
            return clients_data[i][1];
    return -1;
}

void close_queue() {
    int i; for (i = 0; i < client_count; ++i) {
        if (mq_close(clients_data[i][1]) == -1) {
            printf("server: error closing %d client queue\n", i);
        }
        if (kill(clients_data[i][0], SIGINT) == -1) {
            printf("server: error killing %d client\n", i);
        }
    }
    if (queue_descriptor > -1) {
        if(mq_close(queue_descriptor) == -1) {
            printf("server: error closing public queue\n");
        } else {
            printf("server: queue closed\n");
        }

        if (mq_unlink(server_path) == -1) {
            printf("server: error deleting public queue\n");
        } else {
            printf("server: queue deleted successfully\n");
        }
    } else {
        printf("server: there was no need of deleting queue\n");
    }
}

char* convert_time(const time_t *mtime) {
    char* buff = malloc(sizeof(char) * 30);
    struct tm * timeinfo;
    timeinfo = localtime (mtime);
    strftime(buff, 20, "%b %d %H:%M", timeinfo);
    return buff;
}

void int_handler(int _) {
    active = 0;
    exit(2);
}

