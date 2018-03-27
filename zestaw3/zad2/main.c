//
// Created by Michał Osadnik on 22/03/2018.
//


#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/types.h>

#define max_number_of_arguments 64
#define max_number_of_line 256

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("%s", "There's no enough argument! : ¯\\_(ツ)_/¯");
        return 1;
    }
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        printf("%s", "I cannot open this file 🙄");
        return 1;
    }
    char temp_registry[max_number_of_line];
    char *parameters[max_number_of_arguments];
    int argument_number = 0;
    while(fgets(temp_registry, max_number_of_line, file)){
        argument_number = 0;
        while((parameters[argument_number] = strtok(argument_number == 0 ? temp_registry : NULL, " \n\t")) != NULL){
            argument_number++;
            if(argument_number >= max_number_of_arguments){
                printf( "You gave tooo many arguments sir to 🤔:");
                for (int i = 0; i < argument_number; i++) {
                    printf("%s ", parameters[i]);
                }
                return 1;
            }
        };
        pid_t pid = fork();
        if(pid == 0) {
            execvp(parameters[0], parameters);
            exit(1);
        }
        int status;
        wait(&status);
        if (!status) {
            printf( "Error while executing 🤔:");
            for (int i = 0; i < argument_number; i++) {
                printf("%s ", parameters[i]);
            }
            return 1;
        }
    }
    fclose(file);
    return 0;
}