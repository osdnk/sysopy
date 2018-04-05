#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int awaiting_handler = 0;

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
    time_t act_time;
    sigemptyset(&actions.sa_mask);

    while(1){
        sigaction(SIGTSTP, &actions,NULL);
        signal(SIGINT,init_signal);

        if(awaiting_handler)
            continue;

        char buffer[30];
        act_time = time(NULL);
        strftime(buffer, sizeof(buffer),"%H:%M:%S",localtime(&act_time));
        printf("%s\n",buffer);
        sleep(1);
    }
}