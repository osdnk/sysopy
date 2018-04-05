#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int awaiting_handler = 0;
int is_dead_process_handler = 0;
pid_t pid = 0;

void stop_signal_toggle(int sig_num) {
    if(awaiting_handler == 0) {
        printf("\nOdebrano sygnał %d \nOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n", sig_num);
    }
    awaiting_handler = awaiting_handler == 1 ? 0 : 1;
}

void init_signal(int sig_num) {
    printf("\nOdebrano sygnał SIGINT - %d\n", sig_num);
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {

    struct sigaction actions;
    actions.sa_handler = stop_signal_toggle;
    actions.sa_flags = 0;
    sigemptyset(&actions.sa_mask);

    pid = fork();
    if (pid == 0){
        execl("./dater.sh", "./dater.sh", NULL);
        exit(EXIT_SUCCESS);
    }

    while(1){
        sigaction(SIGTSTP, &actions,NULL);
        signal(SIGINT,init_signal);

        if(awaiting_handler == 0) {
            if(is_dead_process_handler){
                is_dead_process_handler = 0;

                pid = fork();
                if (pid == 0){
                    execl("./dater.sh", "./dater.sh", NULL);
                    exit(EXIT_SUCCESS);
                }
            }
        } else {
            if (is_dead_process_handler == 0) {
                kill(pid, SIGKILL);
                is_dead_process_handler = 1;
            }
        }
    }
}