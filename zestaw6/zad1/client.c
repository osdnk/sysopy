#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>

#include "communication.h"

#define FAILURE_EXIT(format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(-1); }

void register_client(key_t privateKey);
void request_mirror(Message *msg);
void request_calc(Message *msg);
void request_time(Message *msg);
void request_end(Message *msg);
int create_queue(char*, int);
void close_queue();
void int_handler(int);

int sessionID = -2;
int queue_descriptor = -1;
int privateID = -1;

// MAIN ////////////////////////////////////////////////////////////////////////

int main() {
    if(atexit(close_queue) == -1)
        FAILURE_EXIT("Registering client's atexit failed");
    if(signal(SIGINT, int_handler) == SIG_ERR)
        FAILURE_EXIT("Registering INT failed");

    char* path = getenv("HOME");
    if (path == NULL)
        FAILURE_EXIT("Getting $HOME failed");

    queue_descriptor = create_queue(path, PROJECT_ID);

    key_t privateKey = ftok(path, getpid());
    if (privateKey == -1)
        FAILURE_EXIT("Generation of private key failed");

    privateID = msgget(privateKey, IPC_CREAT | IPC_EXCL | 0666);
    if (privateID == -1)
        FAILURE_EXIT("Creation of private queue failed");

    register_client(privateKey);

    char cmd[20];
    Message msg;
    while(1) {
        msg.sender_pid = getpid();
        printf("client: enter your request: ");
        if (fgets(cmd, 20, stdin) == NULL){
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
            printf("client: incorrect command\n");
        }
    }
}

void register_client(key_t privateKey) {
    Message msg;
    msg.mtype = LOGIN;
    msg.sender_pid = getpid();
    sprintf(msg.message_text, "%d", privateKey);

    if (msgsnd(queue_descriptor, &msg, MSG_SIZE, 0) == -1)
        FAILURE_EXIT("client: LOGIN request failed\n");
    if (msgrcv(privateID, &msg, MSG_SIZE, 0, 0) == -1)
        FAILURE_EXIT("client: catching LOGIN response failed\n");
    if (sscanf(msg.message_text, "%d", &sessionID) < 1)
        FAILURE_EXIT("client: scanning LOGIN response failed\n");
    if (sessionID < 0)
        FAILURE_EXIT("client: server cannot have more clients\n");

    printf("client: client registered. Session no: %d\n", sessionID);
}

// HANDLERS ////////////////////////////////////////////////////////////////////

void request_mirror(Message *msg) {
    msg->mtype = MIRROR;
    printf("Enter string of characters to Mirror: ");
    if (fgets(msg->message_text, MAX_CONT_SIZE, stdin) == 0) {
        printf("client: too many characters\n");
        return;
    }
    if (msgsnd(queue_descriptor, msg, MSG_SIZE, 0) == -1)
        FAILURE_EXIT("client: MIRROR request failed");
    if (msgrcv(privateID, msg, MSG_SIZE, 0, 0) == -1)
        FAILURE_EXIT("client: catching MIRROR response failed");
    printf("%s", msg->message_text);
}

void request_calc(Message *msg) {
    msg->mtype = CALC;
    printf("Enter expression to calculate: ");
    if (fgets(msg->message_text, MAX_CONT_SIZE, stdin) == 0) {
        printf("client: too many characters\n");
        return;
    }
    if(msgsnd(queue_descriptor, msg, MSG_SIZE, 0) == -1)
        FAILURE_EXIT("client: CALC request failed");
    if(msgrcv(privateID, msg, MSG_SIZE, 0, 0) == -1)
        FAILURE_EXIT("client: catching CALC response failed");
    printf("%s", msg->message_text);
}

void request_time(Message *msg) {
    msg->mtype = TIME;

    if(msgsnd(queue_descriptor, msg, MSG_SIZE, 0) == -1)
        FAILURE_EXIT("client: TIME request failed");
    if(msgrcv(privateID, msg, MSG_SIZE, 0, 0) == -1)
        FAILURE_EXIT("client: catching TIME response failed");
    printf("%s\n", msg->message_text);
}

void request_end(Message *msg) {
    msg->mtype = END;

    if(msgsnd(queue_descriptor, msg, MSG_SIZE, 0) == -1)
        FAILURE_EXIT("client: END request failed");
}

// HELPERS /////////////////////////////////////////////////////////////////////

int create_queue(char *path, int ID) {
    int key = ftok(path, ID);
    if(key == -1) FAILURE_EXIT("Generation of key failed");

    int QueueID = msgget(key, 0);
    if (QueueID == -1) FAILURE_EXIT("Opening queue failed");

    return QueueID;
}

void close_queue() {
    if (privateID > -1) {
        if (msgctl(privateID, IPC_RMID, NULL) == -1){
            printf("There was some error deleting clients's queue\n");
        }
        else {
            printf("Client's queue deleted successfully\n");
        }
    }
}

void int_handler(int _) { exit(2); }