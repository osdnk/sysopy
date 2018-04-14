#define _BSD_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define LINE_MAX 256

int main(int argc, char** argv) {
    if (argc < 3) {
        exit(EXIT_FAILURE);
    }
    int pipe = open("./kk", O_WRONLY);


    char buffer1[LINE_MAX];

    for (int i = 0; i < 5; i++) {
       /* FILE* date = popen("date", "r");
        fgets(buffer1, LINE_MAX, date)*/;
        write(pipe, "komunikat\n", LINE_MAX);
        sleep(1);
    }
    close(pipe);
    return 0;
}