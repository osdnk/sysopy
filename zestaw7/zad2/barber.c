#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>

#include "common.h"

int shared_memory_fd;
sem_t* semaphore;

void handle_signal(int _) {
    printf("Closing barbershop\n");
    exit(0);
}

void invite_client_to_the_chair() {
    pid_t client_pid = barbershop->fifo_queue_of_clients[0];
    barbershop->current_client = client_pid;
    printf("%lo barber: invited client %i\n", current_time(), client_pid);
}

void shave() {
    printf("%lo barber: started shaving client %i\n",
           current_time(),
           barbershop->current_client);

    printf("%lo barber: finished shaving client %i\n",
           current_time(),
           barbershop->current_client);

    barbershop->current_client = 0;
}

void clean_memory() {
    if (semaphore != 0) sem_unlink(PROJECT_PATH);
    if (shared_memory_fd != 0) shm_unlink(PROJECT_PATH);
}

void initialize_barbershop(int argc, char **argv) {
    // Handle signals and atexit() callback
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
    atexit(clean_memory);

    // Control count of arguments
    if (argc < 2) FAIL("Not enough arguments provided\n")

    // Handle room size argument
    int waiting_room_size = (int) strtol(argv[1], 0, 10);
    if (waiting_room_size > BARBER_MAX_QUEUE_SIZE)
        FAIL("Provided room size was too big\n")

    // Create shared memory
    shared_memory_fd = shm_open(
        PROJECT_PATH,
        O_RDWR | O_CREAT | O_EXCL,
        S_IRWXU | S_IRWXG
    );

    if (shared_memory_fd == -1)
        FAIL("Couldn't create shared memory\n")

    // Truncate file
    int error = ftruncate(shared_memory_fd, sizeof(*barbershop));

    if (error == -1)
        FAIL("Failed truncating file\n");

    // Access shared memory
    barbershop = mmap(
        NULL,                   // address
        sizeof(*barbershop),    // length
        PROT_READ | PROT_WRITE, // prot (memory segment security)
        MAP_SHARED,             // flags
        shared_memory_fd,       // file descriptor
        0                       // offset
    );

    if (barbershop == (void*) -1)
        FAIL("Couldn't access shared memory\n")

    semaphore = sem_open(
        PROJECT_PATH,                // path
        O_WRONLY | O_CREAT | O_EXCL, // flags
        S_IRWXU | S_IRWXG,           // mode
        0                            // value
    );

    if (semaphore == (void*) -1)
        FAIL("Couldn't create semaphore\n")

    // Initialize the barbershop
    barbershop->status_of_barber = SLEEPING_INACTIVE;
    barbershop->waiting_room_size = waiting_room_size;
    barbershop->client_count = 0;
    barbershop->current_client = 0;

    // Initialize empty clients fifo_queue_of_clients
    for (int i = 0; i < BARBER_MAX_QUEUE_SIZE; ++i) barbershop->fifo_queue_of_clients[i] = 0;
}

int main(int argc, char** argv) {
    initialize_barbershop(argc, argv);

    release_sem(semaphore);

    while(1) {
        take_sem(semaphore);

        switch (barbershop->status_of_barber) {
            case IDLE:
                if (is_queue_empty()) {
                    printf("%lo barber: barber fell asleep\n", current_time());
                    barbershop->status_of_barber = SLEEPING_INACTIVE;
                } else {
                    // Invite client from fifo_queue_of_clients and then go to ready state
                    // (which will make the barber serve the customer)
                    invite_client_to_the_chair();
                    barbershop->status_of_barber = READY_FOR_SHAVING;
                }
                break;
            case AWAKEN_BY_CLIENT:
                printf("%lo barber: woke up\n", current_time());
                barbershop->status_of_barber = READY_FOR_SHAVING;
                break;
            case SHAVING_CLIENT:
                shave();
                barbershop->status_of_barber = READY_FOR_SHAVING;
                break;
            default:
                break;
        }

        release_sem(semaphore);
    }
}