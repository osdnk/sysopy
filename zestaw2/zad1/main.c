//
// Created by Michał Osadnik on 17/03/2018.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define sys_mode 1
#define lib_mode 0

int generate(char *path, int amount, int len){
    FILE *file = fopen(path, "w+");
    FILE *rnd = fopen("/dev/random", "r");
    char *tmp = malloc(len * sizeof(char) + 1);
    for (int i = 0; i < amount; ++i){
        if(fread(tmp, sizeof(char), (size_t) len+1, rnd) != len+1) {
            return 1;
        }

        for(int j = 0; j < len; ++j) {
            tmp[j] = (char) (abs(tmp[j]) % 25 + 65);
        }

        tmp[len] = 10;

        if(fwrite(tmp, sizeof(char), (size_t) len+1, file) != len+1) {
            return 1;
        }
    }
    fclose(file);
    fclose(rnd);
    free(tmp);
    return 0;
};



void generate_wrapper(char *path, int amount, int len) {
    if (generate(path, amount, len) != 0 ) {
        printf("%s", "Oops! Something went with generating 🦖");
    } 

}


void sort_wrapper(char *path, int amount, int len, int mode) {

}

void copy_wrapper(char *source, char *destination, int amount, int buffer, int mode) {

}

int main(int argc, char **argv) {
    if (argc < 5) {
        printf("%s", "There's no enough argument! : ¯\\_(ツ)_/¯");
        return 1;
    }

    if (strcmp(argv[1], "generate") == 0) {
        int amount = (int) strtol(argv[3], NULL, 10);
        int len = (int) strtol(argv[4], NULL, 10);
        generate_wrapper(argv[2], amount, len);

    } else if (strcmp(argv[1], "sort") == 0) {
        if (argc < 6) {
            printf("%s", "There's no enough argument! : ¯\\_(ツ)_/¯");
            return 1;
        }
        

    } else if (strcmp(argv[1], "copy") == 0) {
        if (argc < 6) {
            printf("%s", "There's no enough argument! : ¯\\_(ツ)_/¯");
            return 1;
        }

    } else {
        printf("%s", "Are you pretty sure everything if ok?🤔");
    }
    return 0;
}
