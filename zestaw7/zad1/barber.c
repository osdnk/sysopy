#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include "shared_utils.h"

int id_of_shared_memory;
int sem_id;

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
    if(sem_id != 0) {
        semctl(sem_id, 0, IPC_RMID);
    }
    if(id_of_shared_memory != 0) {
        shmctl(id_of_shared_memory, IPC_RMID, NULL);
    }
}

void initialize_barbershop(int argc, char **argv) {
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
    atexit(clean_memory);

    if (argc < 2) FAIL("Not enough arguments provided\n")

    int number_of_seats_in_the_barbershop = (int) strtol(argv[1], 0, 10);
    if (number_of_seats_in_the_barbershop > BARBER_MAX_QUEUE_SIZE)
        FAIL("Provided room size was too big\n")

    key_t id_of_the_project = ftok(PROJECT_PATH, PROJECT_ID);
    if (id_of_the_project == -1)
        FAIL("Couldn't obtain a project key\n")

    id_of_shared_memory = shmget(
        id_of_the_project,
        sizeof(struct Barber_info),
        S_IRWXU | IPC_CREAT
    );

    if (id_of_shared_memory == -1)
        FAIL("Couldn't create shared memory\n")

    barbershop = shmat(id_of_shared_memory, 0, 0);
    if (barbershop == (void*) -1)
        FAIL("Couldn't access shared memory\n")

    sem_id = semget(id_of_the_project, 1, IPC_CREAT | S_IRWXU);

    if (sem_id == -1)
        FAIL("Couldn't create semaphore\n")

    semctl(sem_id, 0, SETVAL, 0);

    barbershop->status_of_barber = SLEEPING_INACTIVE;
    barbershop->waiting_room_size = number_of_seats_in_the_barbershop;
    barbershop->client_count = 0;
    barbershop->current_client = 0;

    for (int i = 0; i < BARBER_MAX_QUEUE_SIZE; ++i) barbershop->fifo_queue_of_clients[i] = 0;
}


int main(int argc, char** argv) {
    initialize_barbershop(argc, argv);

    release_sem(sem_id);

    while(1) {
        take_sem(sem_id);

        switch (barbershop->status_of_barber) {
            case IDLE:
                if (is_queue_empty()) {
                    printf("%lo barber: barber fell asleep\n", current_time());
                    barbershop->status_of_barber = SLEEPING_INACTIVE;
                } else {
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

        release_sem(sem_id);
    }
}