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

void register_client();
void request_mirror(struct Message *msg);
void request_calc(struct Message *msg);
void request_time(struct Message *msg);
void request_end(struct Message *msg);
void request_quit(struct Message *msg);
void close_queue();
void int_handler(int);

int sessionID = -2;
mqd_t queue_descriptor = -1;
mqd_t privateID = -1;
char myPath[20];

// MAIN ////////////////////////////////////////////////////////////////////////

int main() {
    if (atexit(close_queue) == -1)
        FAILURE_EXIT("client: registering client's atexit failed\n");
    if (signal(SIGINT, int_handler) == SIG_ERR)
        FAILURE_EXIT("client: registering INT failed!");

    sprintf(myPath, "/%d", getpid());

    queue_descriptor = mq_open(server_path, O_WRONLY);
    if (queue_descriptor == -1) FAILURE_EXIT("Opening public queue failed\n");

    struct mq_attr posixAttr;
    posixAttr.mq_maxmsg = MAX_MESSAGE_QUEUE_SIZE;
    posixAttr.mq_msgsize = MESSAGE_SIZE;

    privateID = mq_open(myPath, O_RDONLY | O_CREAT | O_EXCL, 0666, &posixAttr);
    if (privateID == -1) FAILURE_EXIT("client: creation of private queue failed\n");

    register_client();

    char cmd[20];
    Message msg;
    while(1) {
        msg.sender_pid = getpid();
        printf("Enter your request: ");
        if (fgets(cmd, 20, stdin) == NULL) {
            printf("client: error reading your command\n");
            continue;
        }
        int n = strlen(cmd);
        if (cmd[n-1] == '\n') cmd[n-1] = 0;


        if (strcmp(cmd, "mirror") == 0) {
            request_mirror(&msg);
        } else if (strcmp(cmd, "calc") == 0) {
            request_calc(&msg);
        } else if (strcmp(cmd, "time") == 0) {
            request_time(&msg);
        } else if (strcmp(cmd, "end") == 0) {
            request_end(&msg);
        } else if (strcmp(cmd, "quit") == 0) {
            exit(0);
        } else {
            printf("client: wrong command\n");
        }
    }
}

void register_client() {
    Message msg;
    msg.mtype = LOGIN;
    msg.sender_pid = getpid();

    if (mq_send(queue_descriptor, (char*) &msg, MESSAGE_SIZE, 1) == -1)
        FAILURE_EXIT("client: login request failed\n");
    if (mq_receive(privateID,(char*) &msg, MESSAGE_SIZE, NULL) == -1)
        FAILURE_EXIT("client: catching LOGIN response failed\n");
    if (sscanf(msg.message_text, "%d", &sessionID) < 1)
        FAILURE_EXIT("client: scanning LOGIN response failed\n");
    if (sessionID < 0)
        FAILURE_EXIT("client: server cannot have more clients\n");

    printf("client: client registered! My session nr is %d\n", sessionID);
}

// HANDLERS ////////////////////////////////////////////////////////////////////

void request_mirror(struct Message *msg){
    msg->mtype = MIRROR;
    printf("client: enter string of characters to mirror: ");
    if (fgets(msg->message_text, MAX_CONT_SIZE, stdin) == NULL) {
        printf("client: too many characters\n");
        return;
    }

    if (mq_send(queue_descriptor, (char*) msg, MESSAGE_SIZE, 1) == -1)
        FAILURE_EXIT("client: MIRROR request failed\n");
    if (mq_receive(privateID,(char*) msg, MESSAGE_SIZE, NULL) == -1)
        FAILURE_EXIT("client: catching MIRROR response failed\n");
    printf("%s", msg->message_text);
}

void request_calc(struct Message *msg) {
    msg->mtype = CALC;
    printf("Enter expression to calculate: ");
    if (fgets(msg->message_text, MAX_CONT_SIZE, stdin) == NULL) {
        printf("client: too many characters\n");
        return;
    }
    if (mq_send(queue_descriptor, (char*) msg, MESSAGE_SIZE, 1) == -1)
        FAILURE_EXIT("client: CALC request failed\n");
    if (mq_receive(privateID,(char*) msg, MESSAGE_SIZE, NULL) == -1)
        FAILURE_EXIT("client: catching CALC response failed\n");
    printf("%s", msg->message_text);
}

void request_time(struct Message *msg){
    msg->mtype = TIME;

    if (mq_send(queue_descriptor, (char*) msg, MESSAGE_SIZE, 1) == -1)
        FAILURE_EXIT("client: TIME request failed\n");
    if (mq_receive(privateID,(char*) msg, MESSAGE_SIZE, NULL) == -1)
        FAILURE_EXIT("client: catching TIME response failed\n");
    printf("%s\n", msg->message_text);
}

void request_end(struct Message *msg) {
    msg->mtype = END;

    if (mq_send(queue_descriptor, (char*) msg, MESSAGE_SIZE, 1) == -1)
        FAILURE_EXIT("client: END request failed\n");
}

void request_quit(struct Message *msg) {
    msg->mtype = QUIT;

    if (mq_send(queue_descriptor, (char*) msg, MESSAGE_SIZE, 1) == -1)
        printf("client: END request failed - server may have already been closed\n");
    fflush(stdout);
}

// HELPERS /////////////////////////////////////////////////////////////////////

void close_queue() {
    if (privateID > -1) {
        if (sessionID >= 0) {
            printf("\nBefore quitting, i will try to send QUIT request to public queue!\n");
            Message msg;
            msg.sender_pid = getpid();
            request_quit(&msg);
        }

        if (mq_close(queue_descriptor) == -1) {
            printf("client: there was some error closing servers's queue!\n");
        } else {
            printf("client: servers's queue closed successfully!\n");
        }

        if (mq_close(privateID) == -1) {
            printf("client: there was some error closing client's queue!\n");
        } else {
            printf("client: queue closed successfully!\n");
        }

        if (mq_unlink(myPath) == -1) {
            printf("client: there was some error deleting client's queue!\n");
        } else {
            printf("client: queue deleted successfully!\n");
        }
    } else {
        printf("client: there was no need of deleting queue!\n");
    }
}

void int_handler(int _) { exit(2); }
