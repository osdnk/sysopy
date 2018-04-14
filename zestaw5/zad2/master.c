#define _BSD_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LINE_MAX 256

int main(int argc, char **argv) {
    if (argc < 2) {
        exit(EXIT_FAILURE);
    }
    mkfifo(argv[1], S_IWUSR | S_IRUSR);
    char buffer[LINE_MAX];

    FILE *pipe = fopen(argv[1], "r");
    while (fgets(buffer, LINE_MAX, pipe) != NULL) {
        write(1, buffer, strlen(buffer));
    }
    fclose(pipe);
    return 0;
}