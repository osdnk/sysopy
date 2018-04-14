#define _BSD_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LINE_MAX 256

int main(int argc, char **argv) {
    mkfifo("./kk", S_IWUSR | S_IRUSR);
    char buffer[LINE_MAX];

    FILE *pipe = fopen("./kk", "r");
    while (fgets(buffer, LINE_MAX, pipe) != NULL) {
        printf("%s", buffer);
        //write(STDOUT_FILENO, buffer, strlen(buffer));
    }
    fclose(pipe);
    return 0;
}