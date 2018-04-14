#define _BSD_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LINE_MAX 4

int main(int argc, char **argv) {
    mkfifo("./kk", S_IWUSR | S_IRUSR);
    char buffer[LINE_MAX];

    FILE *pipe = fopen("./kk", "r");
    while (fgets(buffer, LINE_MAX, pipe) != NULL) {
        write(STDOUT_FILENO, buffer, LINE_MAX);
    }
    fclose(pipe);
    return 0;
}