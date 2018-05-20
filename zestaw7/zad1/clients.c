#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include "common.h"


enum Client_status status;
int id_of_shared_memory;
int sem_id;

void initialize_clients() {
    key_t id_of_project = ftok(PROJECT_PATH, PROJECT_ID);
    if (id_of_project == -1)
        FAIL("Error while getting key\n")

    id_of_shared_memory = shmget(id_of_project, sizeof(struct Barber_info), 0);
    if (id_of_shared_memory == -1)
        FAIL("Couldn't create shared memory\n")

    barbershop = shmat(id_of_shared_memory, 0, 0);
    if (barbershop == (void*) -1)
        FAIL("Couldn't access shared memory\n")

    sem_id = semget(id_of_project, 0, 0);
    if (sem_id == -1)
        FAIL("Couldn't create semaphore\n")
}

void take_barbers_chair() {
    pid_t client_pid = getpid();

    if (status == INVITED) {
        pop_queue();
    } else if (status == JUST_ARRIVED) {
        while (1) {
            release_sem(sem_id);
            take_sem(sem_id);
            if (barbershop->status_of_barber == READY_FOR_SHAVING) break;
        }
        status = INVITED;
    }
    barbershop->current_client = client_pid;
    printf("%lo: #%i: took place in the chair\n", current_time(), client_pid);
}

void trigger_barbers_client(int number_of_shaves) {
    pid_t current_clients_pid = getpid();
    int cuts_already_done = 0;

    while (cuts_already_done < number_of_shaves) {
        status = JUST_ARRIVED;

        take_sem(sem_id);

        if (barbershop->status_of_barber == SLEEPING_INACTIVE) {
            printf("%lo #%i: woke up the barber\n", current_time(), current_clients_pid);
            barbershop->status_of_barber = AWAKEN_BY_CLIENT;
            take_barbers_chair();
            barbershop->status_of_barber = SHAVING_CLIENT;
        } else if (!queue_full()) {
            get_in_queue(current_clients_pid);
            printf("%lo #%i: entering the fifo_queue_of_clients\n", current_time(), current_clients_pid);
        } else {
            printf("%lo #%i: could not find place in the fifo_queue_of_clients\n", current_time(), current_clients_pid);
            release_sem(sem_id);
            return;
        }

        release_sem(sem_id);

        while(status != INVITED) {
            take_sem(sem_id);
            if (barbershop->current_client == current_clients_pid) {
                status = INVITED;
                take_barbers_chair();
                barbershop->status_of_barber = SHAVING_CLIENT;
            }
            release_sem(sem_id);
        }

        while(status != SHAVED) {
            take_sem(sem_id);
            if (barbershop->current_client != current_clients_pid) {
                status = SHAVED;
                printf("%lo #%i: shaved\n", current_time(), current_clients_pid);
                barbershop->status_of_barber = IDLE;
                cuts_already_done++;
            }
            release_sem(sem_id);
        }
    }
    printf("%lo #%i: left barbershop after %i cuts_already_done\n", current_time(), current_clients_pid, number_of_shaves);
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
