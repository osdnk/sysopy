#ifndef __COMMON_H__
#define __COMMON_H__

#include <errno.h>
#include <string.h>


#define RED_COLOR "\e[1;31m"
#define RESET_COLOR "\e[0m"

#define FAIL(format, ...) {                                            \
    printf(RED_COLOR format RESET_COLOR, ##__VA_ARGS__);                       \
    exit(-1);                                                                  }


#define PROJECT_PATH "/barbershop10"

#define BARBER_MAX_QUEUE_SIZE 64

enum Barber_status {
    SLEEPING_INACTIVE,
    AWAKEN_BY_CLIENT,
    READY_FOR_SHAVING,
    IDLE,
    SHAVING_CLIENT
};

enum Client_status {
    JUST_ARRIVED,
    INVITED,
    SHAVED
};

struct Barber_info {
    enum Barber_status status_of_barber;
    int client_count;
    int waiting_room_size;
    pid_t current_client;
    pid_t fifo_queue_of_clients[BARBER_MAX_QUEUE_SIZE];
} *barbershop;


long current_time() {
    struct timespec buffer;
    clock_gettime(CLOCK_MONOTONIC, &buffer);
    return buffer.tv_nsec / 1000;
}


void get_sem(sem_t *semaphore) {
    int error = sem_wait(semaphore);
    if (error == -1) FAIL("Semaphore error: %s", strerror(errno))
}

void release_sem(sem_t *semaphore) {
    int error = sem_post(semaphore);
    if (error == -1) FAIL("Semaphore error: %s", strerror(errno))
}


int queue_full() {
    if (barbershop->client_count < barbershop->waiting_room_size) return 0;
    return 1;
}

int is_queue_empty() {
    if (barbershop->client_count == 0) return 1;
    return 0;
}

void get_in_queue(pid_t pid) {
    barbershop->fifo_queue_of_clients[barbershop->client_count] = pid;
    barbershop->client_count += 1;
}

void pop_queue() {
    for (int i = 0; i < barbershop->client_count - 1; ++i) {
        barbershop->fifo_queue_of_clients[i] = barbershop->fifo_queue_of_clients[i + 1];
    }

    barbershop->fifo_queue_of_clients[barbershop->client_count - 1] = 0;
    barbershop->client_count -= 1;
}

#endif
