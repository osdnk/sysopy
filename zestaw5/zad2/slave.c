#define _BSD_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <memory.h>

#define LINE_MAX 256

int main(int argc, char** argv) {
    if (argc < 3) {
        exit(EXIT_FAILURE);
    }
    int pipe = open(argv[1], O_WRONLY);

    char buffer1[LINE_MAX];
    char buffer2[LINE_MAX];

    int it = (int) strtol(argv[2], NULL, 10);
    for (int i = 0; i < it; i++) {
        FILE *date = popen("date", "r");
        fgets(buffer1, LINE_MAX, date);
        int pid = getpid();
        sprintf(buffer2, "Slave: %d - %s", pid, buffer1);
        write(pipe, buffer2, strlen(buffer2));
        sleep(1);
    }
    close(pipe);
    return 0;
}