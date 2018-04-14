#define _BSD_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define LINE_MAX 4

int main(int argc, char** argv) {
    if (argc < 3) {
        exit(EXIT_FAILURE);
    }
    int pipe = open("./kk", O_WRONLY);


    char buffer1[LINE_MAX];

    char * A[5];
    A[0] = "123\n";
    A[1] = "1d3\n";
    A[2] = "323\n";
    A[3] = "f23\n";
    A[4] = "1gg\n";

    for (int i = 0; i < 5; i++) {
        write(pipe, A[i], LINE_MAX);
        sleep(1);
    }
    close(pipe);
    return 0;
}