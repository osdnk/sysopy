//
// Created by Michał Osadnik on 14/04/2018.
//

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/types.h>



#define max_number_of_commands 64
#define max_number_of_line 256


char* trim_white(char *origStr){
    char* buffer = malloc(sizeof(char) * 200);
    char* i = origStr;
    while(*i == ' ') i++;
    int j = 0;
    while(*i != 0){
        while((*i != ' ') && (*i != 0)){
            buffer[j++] = *i;
            i++;
        }
        if(*i == ' '){
            while(*i == ' ') i++;
            if (*i != 0)
                buffer[j++] = ' ';
        }
    }
    buffer[j+1] = 0;
    return buffer;
}
char** parse_program_arguments(char *line){
    int size = 0;
    char** args = NULL;
    char delimiters[3] = {' ','\n','\t'};
    char* a = strtok(line, delimiters);
    while(a != NULL){
        size++;
        args = realloc(args, sizeof(char*) * size);
        if(args == NULL){
            exit(EXIT_FAILURE);
        }
        args[size-1] = a;
        a = strtok(NULL, delimiters);
    }
    args = realloc(args, sizeof(char*) * (size+1));
    if(args == NULL){
        exit(EXIT_FAILURE);
    }
    args[size] = NULL;

    return args;
}


int execute_line(char * parameters) {
    char * tmp_reg;
    int command_number = 0;
    int pipes[100][2];


    while((tmp_reg = strtok(command_number == 0 ? parameters : NULL, "|")) != NULL){
        pid_t cp = fork();
        if (cp == 0) {
            if (command_number == 0) {
                dup2(pipes[0][1], STDOUT_FILENO);
            }

            if (command_number == 1) {
                dup2(pipes[0][0], STDIN_FILENO);
            }

            char ** exec_params = parse_program_arguments(trim_white(tmp_reg));
            if(execvp(exec_params[0], exec_params) == -1) {
                printf("Error occured on execution: ");
                for(int j = 0; exec_params[j] != NULL; j++) printf("%s ", exec_params[j]);
                printf("\n");
            }

            // close(pipes[0][1]);

            exit(EXIT_SUCCESS);
        }
        wait(NULL);
        command_number++;
    };

    printf("%s", "213");
    exit(0);
}


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
    char *parameters[max_number_of_commands];
    int argument_number = 0;
    while(fgets(temp_registry, max_number_of_line, file)){
        argument_number = 0;
        /*while((parameters[argument_number] = strtok(argument_number == 0 ? temp_registry : NULL, " \n\t")) != NULL){
            argument_number++;
            if(argument_number >= max_number_of_commands){
                printf( "You gave tooo many arguments sir to 🤔:");
                for (int i = 0; i < argument_number; i++) {
                    printf("%s ", parameters[i]);
                }
                return 1;
            }
        };*/
        pid_t pid = fork();
        if(pid == 0) {
            execute_line(temp_registry);
            //execvp(parameters[0], parameters);
            exit(1);
        }
        int status;
        wait(&status);
        if (status) {
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