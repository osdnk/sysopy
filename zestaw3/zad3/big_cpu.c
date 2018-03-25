//
// Created by Michał Osadnik on 25/03/2018.
//
#include <stdlib.h>
#include <stdio.h>

void horse_fast_rider();

void horse_faster_rider() {
    horse_fast_rider();
}

void horse_fast_rider() {
    printf("Oh my God, staph, sir!");
    horse_fast_rider();
    horse_fast_rider();
}

int main() {
    horse_fast_rider();
}