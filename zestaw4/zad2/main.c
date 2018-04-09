#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <memory.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
#define WRITE_MSG(format, ...) { char buffer[255]; sprintf(buffer, format, ##__VA_ARGS__); write(1, buffer, strlen(buffer));}

void intHandler(int, siginfo_t *, void *);

void usrHandler(int, siginfo_t *, void *);

void chldHandler(int, siginfo_t *, void *);

void rtHandler(int, siginfo_t *, void *);

volatile int N;
volatile int K;
volatile int n;
volatile int k;
volatile pid_t *childrenArr;
volatile pid_t *awaitingArr;


int checkIfChildren(pid_t pid) {
    for (int i = 0; i < N; i++)
        if (childrenArr[i] == pid) return i;
    return -1;
}

void removeChild(pid_t pid) {
    for (int i = 0; i < N; i++)
        if (childrenArr[i] == pid) {
            childrenArr[i] = -1;
            return;
        }
}

int main(int argc, char *argv[]) {
    if (argc < 3) FAILURE_EXIT(1, "Wrong execution. Use ./main VAL_N VAL_K\n");
    N = (int) strtol(argv[1], '\0', 10);
    K = (int) strtol(argv[2], '\0', 10);
    if (N < 1) FAILURE_EXIT(1, "Wrong N\n");
    if (K < 1) FAILURE_EXIT(1, "Wrong K\n");
    if (N < K) FAILURE_EXIT(1, "K can't be larger than N\n");

    childrenArr = calloc((size_t) N, sizeof(pid_t));
    awaitingArr = calloc((size_t) N, sizeof(pid_t));
    n = k = 0;

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    act.sa_sigaction = intHandler;
    if (sigaction(SIGINT, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGINT\n")

    act.sa_sigaction = usrHandler;
    if (sigaction(SIGUSR1, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGUSR1\n")

    act.sa_sigaction = chldHandler;
    if (sigaction(SIGCHLD, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGCHLD\n")

    for (int i = SIGRTMIN; i <= SIGRTMAX; i++) {
        act.sa_sigaction = rtHandler;
        if (sigaction(i, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGRTMIN+%d\n", i - SIGRTMIN);
    }

    for (int i = 0; i < N; i++) {
        pid_t pid = fork();
        if (!pid) {
            execl("./child", "./child", NULL);
            FAILURE_EXIT(2, "Error creating child process\n");
        } else {
            childrenArr[n++] = pid;
        }
    }

    while (wait(NULL))
        if (errno == ECHILD) FAILURE_EXIT(2, "ERROR CHILD\n");
}

void intHandler(int signum, siginfo_t *info, void *context) {
    WRITE_MSG("\rMother: Received SIGINT\n");

    for (int i = 0; i < N; i++)
        if (childrenArr[i] != -1) {
            kill(childrenArr[i], SIGKILL);
            waitpid(childrenArr[i], NULL, 0);
        }

    exit(0);
}

void usrHandler(int signum, siginfo_t *info, void *context) {
    WRITE_MSG("Mother: Received SIGUSR1 form PID: %d\n", info->si_pid);

    if (checkIfChildren(info->si_pid) == -1)return;

    if (k >= K) {
        WRITE_MSG("Mother: Sending SIGUSR1 to Child PID: %d\n", info->si_pid);
        kill(info->si_pid, SIGUSR1);
        waitpid(info->si_pid, NULL, 0);
    } else {
        awaitingArr[k++] = info->si_pid;
        if (k >= K) {
            for (int i = 0; i < K; i++) {
                if (awaitingArr[i] > 0) {
                    WRITE_MSG("Mother: Sending SIGUSR1 to Child PID: %d\n", awaitingArr[i]);
                    kill(awaitingArr[i], SIGUSR1);
                    waitpid(awaitingArr[i], NULL, 0);
                }
            }
        }
    }
}

void chldHandler(int signum, siginfo_t *info, void *context) {
    if (info->si_code == CLD_EXITED) {
        WRITE_MSG("Mother: Child %d has terminated, with exit status: %d\n", info->si_pid, info->si_status);
    } else {
        WRITE_MSG("Mother: Child %d has terminated by signal: %d\n", info->si_pid, info->si_status);
    }
    n--;
    if (n == 0) {
        WRITE_MSG("Mother: No more children, Terminating\n");
        exit(0);
    }
    removeChild(info->si_status);
}

void rtHandler(int signum, siginfo_t *info, void *context) {
    WRITE_MSG("Mother: Received SIGRT: SIGMIN+%i, for PID: %d\n", signum - SIGRTMIN, info->si_pid);
}