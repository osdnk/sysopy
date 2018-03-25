//
// Created by Michał Osadnik on 22/03/2018.
//


#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/time.h>


#define max_number_of_arguments 64
#define max_number_of_line 256


int handle_limits(char *time, char *memory) {
    unsigned long int time_limit = strtol(time, NULL, 10);
    struct rlimit r_limit_cpu;
    r_limit_cpu.rlim_max = (rlim_t) time_limit;
    r_limit_cpu.rlim_cur = (rlim_t) time_limit;
    if (setrlimit(RLIMIT_CPU, &r_limit_cpu) != 0) {
        printf("I cannot set this limit cpu 🙅");
        return -1;
    }

    unsigned long int memory_limit = strtol(memory, NULL, 10);
    struct rlimit r_limit_memory;
    r_limit_memory.rlim_max = (rlim_t) memory_limit * 1024 * 1024;
    r_limit_memory.rlim_cur = (rlim_t) memory_limit * 1024 * 1024;

    if (setrlimit(RLIMIT_DATA, &r_limit_memory) != 0) {
        printf("I cannot set this limit memory 🙅");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("%s", "There's no enough argument! Dont u forgot about limits, dear? : ¯\\_(ツ)_/¯");
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("%s", "I cannot open this file 🙄");
        return 1;
    }
    struct rusage prev_usage;
    getrusage(RUSAGE_CHILDREN, &prev_usage);
    char temp_registry[max_number_of_line];
    char *parameters[max_number_of_arguments];
    int argument_number = 0;
    while (fgets(temp_registry, max_number_of_line, file)) {
        argument_number = 0;
        while ((parameters[argument_number] = strtok(argument_number == 0 ? temp_registry : NULL, " \n\t")) != NULL) {
            argument_number++;
            if (argument_number >= max_number_of_arguments) {
                fprintf(stderr, "You gave tooo many arguments sir to %s 🤔", parameters[0]);
            }
        };
        pid_t pid = fork();
        if (pid == 0) {
            handle_limits(argv[2], argv[3]);
            execvp(parameters[0], parameters);
            exit(1);
        }
        int status;
        wait(&status);
        if (status) {
            printf("Error while running command: ");
        }
        struct rusage usage;
        getrusage(RUSAGE_CHILDREN, &usage);
        struct timeval ru_utime;
        struct timeval ru_stime;
        timersub(&usage.ru_utime, &prev_usage.ru_utime, &ru_utime);
        timersub(&usage.ru_stime, &prev_usage.ru_stime, &ru_stime);
        prev_usage = usage;
        for (int i = 0; i < argument_number; i++) {
            printf("%s ", parameters[i]);
        }
        printf("\n");
        printf("User CPU time used: %d.%d seconds,  system CPU time used: %d.%d seconds\n\n", (int) ru_utime.tv_sec,
               (int) ru_utime.tv_usec, (int) ru_stime.tv_sec, (int) ru_stime.tv_usec);
    }
    fclose(file);
    return 0;
}
