//
// Created by Michał Osadnik on 25/03/2018.
//
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void horse_fast_rider();

void horse_faster_rider() {
    horse_fast_rider();
}

void horse_fast_rider() {
    printf("Oh my God, staph, sir!\n");
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 5000;
    nanosleep(&tim , &tim2);
    horse_fast_rider();
    horse_fast_rider();
}

int main() {
    horse_fast_rider();
}