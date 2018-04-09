//
// Created by Michał Osadnik on 09/04/2018.
//

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <memory.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
#define WRITE_MSG(format, ...) { char buffer[255]; sprintf(buffer, format, ##__VA_ARGS__); write(1, buffer, strlen(buffer));}

void childHandler(int, siginfo_t *, void *);

void motherHandler(int, siginfo_t *, void *);

void childProcess();

void motherProcess();

volatile int L;
volatile int TYPE;
volatile int sentToChild = 0;
volatile int receivedByChild = 0;
volatile int receivedFromChild = 0;
volatile pid_t child;

void printStats() {
    printf("Signals sent: %d\n", sentToChild);
    printf("Signals received from child: %d\n", receivedFromChild);
}

int main(int argc, char *argv[]) {

    if (argc < 3) FAILURE_EXIT(1, "Wrong execution. Use ./main VAL_L VAL_TYPE\n");
    L = (int) strtol(argv[1], '\0', 10);
    TYPE = (int) strtol(argv[2], '\0', 10);

    if (L < 1) FAILURE_EXIT(1, "Wrong L Argument\n")
    if (TYPE < 1 || TYPE > 3) FAILURE_EXIT(1, "Wrong Type Argument\n")

    child = fork();
    if (!child) childProcess();
    else if (child > 0) motherProcess();
    else FAILURE_EXIT(2, "Error while Forking\n");

    printStats();

    return 0;
}

void childHandler(int signum, siginfo_t *info, void *context) {
    if (signum == SIGINT) {
        sigset_t mask;
        sigfillset(&mask);
        sigprocmask(SIG_SETMASK, &mask, NULL);
        WRITE_MSG("Signals received by child: %d\n", receivedByChild);
        exit((unsigned) receivedByChild);
    }
    if (info->si_pid != getppid()) return;

    if (TYPE == 1 || TYPE == 2) {
        if (signum == SIGUSR1) {
            receivedByChild++;
            kill(getppid(), SIGUSR1);
            WRITE_MSG("Child: SIGUSR1 received and sent back\n")
        } else if (signum == SIGUSR2) {
            receivedByChild++;
            WRITE_MSG("Child: SIGUSR2 received Terminating\n")
            WRITE_MSG("Signals received by child: %d\n", receivedByChild);
            exit((unsigned) receivedByChild);
        }
    } else if (TYPE == 3) {
        if (signum == SIGRTMIN) {
            receivedByChild++;
            kill(getppid(), SIGRTMIN);
            WRITE_MSG("Child: SIGRTMIN received and sent back\n")
        } else if (signum == SIGRTMAX) {
            receivedByChild++;
            WRITE_MSG("Child: SIGRTMAX received Terminating\n")
            WRITE_MSG("Signals received by child: %d\n", receivedByChild);
            exit((unsigned) receivedByChild);
        }
    }
}


void motherHandler(int signum, siginfo_t *info, void *context) {
    if (signum == SIGINT) {
        WRITE_MSG("Mother: Received SIGINT\n");
        kill(child, SIGUSR2);
        printStats();
        exit(9);
    }
    if (info->si_pid != child) return;

    if ((TYPE == 1 || TYPE == 2) && signum == SIGUSR1) {
        receivedFromChild++;
        WRITE_MSG("Mother: Received SIGUSR1 form Child\n");
    } else if (TYPE == 3 && signum == SIGRTMIN) {
        receivedFromChild++;
        WRITE_MSG("Mother: Received SIGRTMIN from Child\n");
    }
}

void childProcess() {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = childHandler;

    if (sigaction(SIGINT, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGINT\n");
    if (sigaction(SIGUSR1, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGUSR1\n");
    if (sigaction(SIGUSR2, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGUSR2\n");
    if (sigaction(SIGRTMIN, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGRTMIN\n");
    if (sigaction(SIGRTMAX, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGRTMAX\n");

    while (1) {
        sleep(1);
    }
}

void motherProcess() {
    sleep(1);

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = motherHandler;

    if (sigaction(SIGINT, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGINT\n");
    if (sigaction(SIGUSR1, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGUSR1\n");
    if (sigaction(SIGRTMIN, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGRTMIN\n");

    if (TYPE == 1 || TYPE == 2) {
        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGINT);
        for (; sentToChild < L; sentToChild++) {
            WRITE_MSG("Mother: Sending SIGUSR1\n");
            kill(child, SIGUSR1);
            if (TYPE == 2) sigsuspend(&mask);
        }
        WRITE_MSG("Mother: Sending SIGUSR2\n");
        kill(child, SIGUSR2);
    } else if (TYPE == 3) {
        for (; sentToChild < L; sentToChild++) {
            WRITE_MSG("Mother: Sending SIGRTMIN\n");
            kill(child, SIGRTMIN);
        }
        sentToChild++;
        WRITE_MSG("Mother: Sending SIGRTMAX\n");
        kill(child, SIGRTMAX);
    }

    int status = 0;
    waitpid(child, &status, 0);
    if (WIFEXITED(status))
        receivedByChild = WEXITSTATUS(status);
    else FAILURE_EXIT(1, "Error with termination of Child!\n");
}