#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>

#include "common.h"


enum Client_status status;
int shared_memory_fd;
sem_t* semaphore;


void initialize_clients() {
    shared_memory_fd = shm_open(PROJECT_PATH, O_RDWR, S_IRWXU | S_IRWXG);

    if (shared_memory_fd == -1)
        FAIL("Couldn't create shared memory\n")

    // Truncate file
    int error = ftruncate(shared_memory_fd, sizeof(*barbershop));

    if (error == -1)
        FAIL("Failed truncating file\n");

    barbershop = mmap(NULL,
                      sizeof(*barbershop),
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED,
                      shared_memory_fd,
                      0);

    if (barbershop == (void*) -1)
        FAIL("Couldn't access shared memory\n")

    semaphore = sem_open(PROJECT_PATH, O_WRONLY, S_IRWXU | S_IRWXG, 0);

    if (semaphore == (void*) -1)
        FAIL("Couldn't create semaphore\n")
}

void take_barbers_chair() {
    pid_t client_pid = getpid();

    if (status == INVITED) {
        pop_queue();
    } else if (status == JUST_ARRIVED) {
        while (1) {
            release_sem(semaphore);
            get_sem(semaphore);
            if (barbershop->status_of_barber == READY_FOR_SHAVING) break;
        }
        status = INVITED;
    }
    barbershop->current_client = client_pid;
    printf("%lo: #%i: took place in the chair\n", current_time(), client_pid);
}

void trigger_barbers_client(int number_of_shaves) {
    pid_t current_client_pid = getpid();
    int cuts_already_done = 0;

    while (cuts_already_done < number_of_shaves) {
        status = JUST_ARRIVED;

        get_sem(semaphore);

        if (barbershop->status_of_barber == SLEEPING_INACTIVE) {
            printf("%lo #%i: woke up the barber\n", current_time(), current_client_pid);
            barbershop->status_of_barber = AWAKEN_BY_CLIENT;
            take_barbers_chair();
            barbershop->status_of_barber = SHAVING_CLIENT;
        } else if (!queue_full()) {
            get_in_queue(current_client_pid);
            printf("%lo #%i: entering the fifo_queue_of_clients\n", current_time(), current_client_pid);
        } else {
            printf("%lo #%i: could not find place in the fifo_queue_of_clients\n", current_time(), current_client_pid);
            release_sem(semaphore);
            return;
        }

        release_sem(semaphore);

        while(status < INVITED) {
            get_sem(semaphore);
            if (barbershop->current_client == current_client_pid) {
                status = INVITED;
                take_barbers_chair();
                barbershop->status_of_barber = SHAVING_CLIENT;
            }
            release_sem(semaphore);
        }

        while(status < SHAVED) {
            get_sem(semaphore);
            if (barbershop->current_client != current_client_pid) {
                status = SHAVED;
                printf("%lo #%i: shaved\n", current_time(), current_client_pid);
                barbershop->status_of_barber = IDLE;
                cuts_already_done++;
            }
            release_sem(semaphore);
        }
    }
    printf("%lo #%i: left barbershop after %i cuts_already_done\n", current_time(), current_client_pid, number_of_shaves);
}

int main(int argc, char** argv) {
    if (argc < 3) FAIL("Not enough arguments provided\n")
    int number_of_clients_to_be_spawned = (int) strtol(argv[1], 0, 10);
    int number_of_shaves_to_be_done = (int) strtol(argv[2], 0, 10);
    initialize_clients();

    for (int i = 0; i < number_of_clients_to_be_spawned; i++) {
        if (!fork()) {
            trigger_barbers_client(number_of_shaves_to_be_done);
            exit(0);
        }
    }
    while (wait(0)) if (errno != ECHILD) break;
}
