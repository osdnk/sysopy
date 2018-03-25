//
// Created by Michał Osadnik on 25/03/2018.
//
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void horse_fast_rider();

void horse_faster_rider(int i) {
    int n = (i + 1) % 10000;
    if (n == 0) {
        printf("Pls, staph, sir!\n");
    }
    horse_fast_rider(n);
}

void horse_fast_rider(int i) {
    int *arr = malloc(100000 * sizeof(int));
    for (int j=0; j<100000; j++) {
	    arr[j]=j;
    }
    free(arr);
    horse_faster_rider(i+1);
}

int main() {
    horse_fast_rider(0);
}
