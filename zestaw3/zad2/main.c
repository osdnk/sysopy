//
// Created by Michał Osadnik on 22/03/2018.
//


#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <memory.h>
#include <stdlib.h>

#define ARGS_MAX 64
#define LINE_MAX 256

int main(int argc, char const *argv[]) {
    if(argc < 2) exit(1);
    FILE* file = fopen(argv[1], "r");
    if(!file) exit(1);
    char r0[LINE_MAX];
    char *args[ARGS_MAX];
    int argNum = 0;
    while(fgets(r0, LINE_MAX, file)){
        argNum = 0;
        while((args[argNum++] = strtok(argNum == 0 ? r0 : NULL, " \n\t")) != NULL){
            if(argNum + 1 >= ARGS_MAX){
                fprintf(stderr, "Command exceeds maximum number (%d) of arguments: %s\nWith arguments:", ARGS_MAX , args[0]);
                for(int i = 1; i < argNum; i++) fprintf(stderr, " %s", args[i]);
                fprintf(stderr, "\n");
            }
        };
        pid_t pid = fork();
        if(!pid) {
            execvp(args[0], args);
        }
        int status;
        wait(&status);
        if(status){
            fprintf(stderr, "Error while running command: %s\nWith arguments:", args[0]);
            for(int i = 1; i < argNum; i++) fprintf(stderr, " %s", args[i]);
            fprintf(stderr, "\n");
        }
    }
    fclose(file);
    return 0;
}