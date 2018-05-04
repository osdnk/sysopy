#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#define MAX_CLIENTS  10
#define PROJECT_ID 0x099
#define MAX_CONT_SIZE 4096

typedef enum mtype {
    LOGIN = 1, MIRROR = 2, CALC = 3, TIME = 4, END = 5, INIT = 6
} mtype;

typedef struct Message {
    long mtype;
    pid_t sender_pid;
    char message_text[MAX_CONT_SIZE];
} Message;

// msgsz does not contain mtype
const size_t MSG_SIZE = sizeof(Message) - sizeof(long);

#endif