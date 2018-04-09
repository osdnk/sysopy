#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void usrHandler(int signum) {
    kill(getppid(), SIGRTMIN + (rand() % (SIGRTMAX - SIGRTMIN)));
}

int main() {
    signal(SIGUSR1, usrHandler);
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);

    srand((unsigned int) getpid());
    int sleepTime = (rand() % 11);

    printf("Hi, Im %d, sleeping for %d\n", getpid(), sleepTime);
    fflush(stdout);
    sleep((unsigned int) sleepTime);

    kill(getppid(), SIGUSR1);

    sigsuspend(&mask);

    return sleepTime;
}