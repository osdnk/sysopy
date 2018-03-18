//
// Created by Michał Osadnik on 17/03/2018.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define sys_mode 1
#define lib_mode 0

int generate(char *path, int amount, int len) {
    FILE *file = fopen(path, "w+");
    FILE *rnd = fopen("/dev/random", "r");
    char *tmp = malloc(len * sizeof(char) + 1);
    for (int i = 0; i < amount; ++i) {
        if (fread(tmp, sizeof(char), (size_t) len + 1, rnd) != len + 1) {
            return 1;
        }

        for (int j = 0; j < len; ++j) {
            tmp[j] = (char) (abs(tmp[j]) % 25 + 65);
        }

        tmp[len] = 10;

        if (fwrite(tmp, sizeof(char), (size_t) len + 1, file) != len + 1) {
            return 1;
        }
    }
    fclose(file);
    fclose(rnd);
    free(tmp);
    return 0;
};


void generate_wrapper(char *path, int amount, int len) {
    if (generate(path, amount, len) != 0) {
        printf("%s", "Oops! Something went with generating 🦖");
    }

}

int lib_sort(char *path, int amount, int len) {
    FILE *file = fopen(path, "r+");
    char *reg1 = malloc((len + 1) * sizeof(char));
    char *reg2 = malloc((len + 1) * sizeof(char));

    long int offset = (long int) ((len + 1) * sizeof(char));

    for (int i = 0; i < amount; i++) {
        fseek(file, i * offset, 0);
        if (fread(reg1, sizeof(char), (size_t)(len + 1), file) != (len + 1)) {
            return 1;
        }

        for (int j = 0; j < i; j++) {
            fseek(file, j * offset, 0);
            if (fread(reg2, sizeof(char), (size_t)(len + 1), file) != (len + 1)) {
                return 1;
            }
            if (reg2[0] > reg1[0]) {
                fseek(file, j * offset, 0);
                if (fwrite(reg1, sizeof(char), (size_t)(len + 1), file) != (len + 1)) {
                    return 1;
                }
                fseek(file, i * offset, 0);
                if (fwrite(reg2, sizeof(char), (size_t)(len + 1), file) != (len + 1)) {
                    return 1;
                }
                char *tmp = reg1;
                reg1 = reg2;
                reg2 = tmp;
            }
        }
    }

    fclose(file);
    free(reg1);
    free(reg2);
    return 0;
};

int sys_sort(char *path, int amount, int len) {
    int file = open(path, O_RDWR);
    char *reg1 = malloc((len + 1) * sizeof(char));
    char *reg2 = malloc((len + 1) * sizeof(char));
    long int offset = (long int) ((len + 1) * sizeof(char));

    for (int i = 0; i < amount; i++) {
        lseek(file, i * offset, SEEK_SET);

        if (read(file, reg1, (size_t)(len + 1) * sizeof(char)) != (len + 1)) {
            return 1;
        }

        for (int j = 0; j < i; j++) {
            lseek(file, j * offset, SEEK_SET);
            if (read(file, reg2, sizeof(char) * (len + 1)) != (len + 1)) {
                return 1;
            }
            if (reg2[0] > reg1[0]) {
                lseek(file, j * offset, 0);
                if (write(file, reg1, sizeof(char) * (len + 1)) != (len + 1)) {
                    return 1;
                }
                lseek(file, i * offset, 0);
                if (write(file, reg2, sizeof(char) * (len + 1)) != (len + 1)) {
                    return 1;
                }
                char *tmp = reg1;
                reg1 = reg2;
                reg2 = tmp;
            }
        }
    }

    close(file);
    free(reg1);
    free(reg2);
    return 0;
}


void sort_wrapper(char *path, int amount, int len, int mode) {
    if (mode == lib_mode) {
        if (lib_sort(path, amount, len) == 1) {
            printf("%s", "Oops! Something went with sorting 🦖");
        }
    } else {
        if (sys_sort(path, amount, len) == 1) {
            printf("%s", "Oops! Something went with sorting 🦖");
        }

    }

}

int sys_copy(char *path, char *dest, int amount, int len){
    int source = open(path, O_RDONLY);
    int target = open(dest, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    char *tmp = malloc(len * sizeof(char));

    for (int i = 0; i < amount; i++){
        if(read(source, tmp, (size_t) (len + 1) * sizeof(char)) != (len + 1)) {
            return 1;
        }

        if(write(target, tmp, (size_t) (len + 1) * sizeof(char)) != (len + 1)) {
            return 1;
        }
    }
    close(source);
    close(target);
    free(tmp);
    return 0;
};

int lib_copy(char *path, char *dest, int amount, int len) {
    FILE *source = fopen(path, "r");
    FILE *target = fopen(dest, "w+");
    char *tmp = malloc(len * sizeof(char));

    for (int i = 0; i < amount; i++){
        if(fread(tmp, sizeof(char), (size_t) (len + 1), source) != (len + 1)) {
            return 1;
        }

        if(fwrite(tmp, sizeof(char), (size_t) (len + 1), target) != (len + 1)) {
            return 1;
        }
    }
    fclose(source);
    fclose(target);
    free(tmp);
    return 0;
};

void copy_wrapper(char *source, char *destination, int amount, int buffer, int mode) {
    if (mode == lib_mode) {
        if (lib_copy(source, destination, amount, buffer) == 1) {
            printf("%s", "Oops! Something went with copying 🦖");
        }
    } else {
        if (sys_copy(source, destination, amount, buffer) == 1) {
            printf("%s", "Oops! Something went with copying 🦖");
        }

    }
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
        int amount = (int) strtol(argv[3], NULL, 10);
        int len = (int) strtol(argv[4], NULL, 10);
        if (strcmp(argv[5], "sys") == 0) {
            sort_wrapper(argv[2], amount, len, sys_mode);
        } else if (strcmp(argv[5], "lib") == 0) {
            sort_wrapper(argv[2], amount, len, lib_mode);
        } else {
            printf("%s", "There's no such a mode 🙄");
        }
    } else if (strcmp(argv[1], "copy") == 0) {
        if (argc < 7) {
            printf("%s", "There's no enough argument! : ¯\\_(ツ)_/¯");
            return 1;
        }
        int amount = (int) strtol(argv[4], NULL, 10);
        int len = (int) strtol(argv[5], NULL, 10);
        if (strcmp(argv[6], "sys") == 0) {
            copy_wrapper(argv[2], argv[3], amount, len, sys_mode);
        } else if (strcmp(argv[6], "lib") == 0) {
            copy_wrapper(argv[2], argv[3], amount, len, lib_mode);

        } else {
            printf("%s", "There's no such a mode 🙄");
        }

    } else {
        printf("%s", "Are you pretty sure everything if ok?🤔");
    }
    return 0;
}
