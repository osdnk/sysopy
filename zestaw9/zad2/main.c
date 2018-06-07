#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <signal.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

char **buffer;
sem_t *b_sem;
int P, K, N, L, nk, search, verbose;
FILE *file;
char file_name[FILENAME_MAX];
int production_index = 0, consumption_index = 0;
int finished = 0;
pthread_t *p_threads;
pthread_t *k_threads;

void sig_hanlder(int signo) {
    fprintf(stderr, "Received signal %s, Canceling threads\n", signo == SIGALRM ? "SIGALRM" : "SIGINT");
    for (int p = 0; p < P; p++)
        pthread_cancel(p_threads[p]);
    for (int k = 0; k < K; k++)
        pthread_cancel(k_threads[k]);
    exit(EXIT_SUCCESS);
}

void configurate(char *config_path) {
    FILE *config;
    if ((config = fopen(config_path, "r")) == NULL) FAILURE_EXIT(2, "Opening config file failed");
    fscanf(config, "%d %d %d %s %d %d %d %d", &P, &K, &N, file_name, &L, &search, &verbose, &nk);
    printf("CONFIGURATION\nP: %d\nK: %d\nN: %d\nFile Name: %s\nL: %d\nSearch Mode: %d\nVerbose: %d\nnk: %d\n", P, K, N,
           file_name, L, search, verbose, nk);
    fclose(config);
}

void __init__() {
    signal(SIGINT, sig_hanlder);
    if (nk > 0) signal(SIGALRM, sig_hanlder);

    if ((file = fopen(file_name, "r")) == NULL) FAILURE_EXIT(2, "Opening file failed");

    buffer = calloc((size_t) N, sizeof(char *));

    b_sem = malloc((N + 3) * sizeof(sem_t));
    for (int i = 0; i < N + 2; ++i)
        sem_init(&b_sem[i], 0, 1);
    sem_init(&b_sem[N+2], 0, (unsigned int) N);

    p_threads = malloc(P * sizeof(pthread_t));
    k_threads = malloc(K * sizeof(pthread_t));
}

void __del__() {
    if (file) fclose(file);

    for (int i = 0; i < N; ++i)
        if (buffer[i]) free(buffer[i]);
    free(buffer);

    for (int j = 0; j < N + 4; ++j)
        sem_destroy(&b_sem[j]);
    free(b_sem);
}

int length_search(int line_length){
    return search == (line_length > L ? 1 : line_length < L ? -1 : 0);
}

void *producer(void *pVoid) {
    int index;
    char line[LINE_MAX];
    while (fgets(line, LINE_MAX, file) != NULL) {
        if(verbose) fprintf(stderr, "Producer[%ld]: taking file line\n", pthread_self());
        sem_wait(&b_sem[N]);

        sem_wait(&b_sem[N+2]);

        index = production_index;
        if(verbose) fprintf(stderr, "Producer[%ld]: taking buffer index (%d)\n",  pthread_self(), index);
        production_index = (production_index + 1) % N;


        sem_wait(&b_sem[index]);
        sem_post(&b_sem[N]);

        buffer[index] = malloc((strlen(line) + 1) * sizeof(char));
        strcpy(buffer[index], line);
        if(verbose) fprintf(stderr, "Producer[%ld]: line copied to buffer at index (%d)\n",  pthread_self(), index);

        sem_post(&b_sem[index]);
    }
    if(verbose) fprintf(stderr, "Producer[%ld]: Finished\n", pthread_self());
    return NULL;
}

void *consumer(void *pVoid) {
    char *line;
    int index;
    while (1) {
        sem_wait(&b_sem[N+1]);
        while (buffer[consumption_index] == NULL) {
            sem_post(&b_sem[N+1]);
            if(finished){
                if(verbose) fprintf(stderr, "Consumer[%ld]: Finished \n",  pthread_self());
                return NULL;
            }
            sem_wait(&b_sem[N+1]);
        }

        index = consumption_index;
        if(verbose) fprintf(stderr, "Consumer[%ld]: taking buffer index (%d)\n",  pthread_self(), index);
        consumption_index = (consumption_index + 1) % N;

        sem_wait(&b_sem[index]);

        line = buffer[index];
        buffer[index] = NULL;
        if(verbose) fprintf(stderr, "Consumer[%ld]: taking line from buffer at index (%d)\n",  pthread_self(), index);

        sem_post(&b_sem[N+2]);
        sem_post(&b_sem[N + 1]);
        sem_post(&b_sem[index]);

        if(length_search((int) strlen(line))){
            if(verbose) fprintf(stderr, "Consumer[%ld]: found line with length %d %c %d\n",
                    pthread_self(), (int) strlen(line), search == 1 ? '>' : search == -1 ? '<' : '=', L);
            fprintf(stderr, "Consumer[%ld]: Index(%d), %s",  pthread_self(), index, line);
        }
        free(line);
    }
}

void start_threads() {
    for (int p = 0; p < P; ++p)
        pthread_create(&p_threads[p], NULL, producer, NULL);
    for (int k = 0; k < K; ++k)
        pthread_create(&k_threads[k], NULL, consumer, NULL);
    if (nk > 0) alarm(nk);
}

void join_threads(){
    for (int p = 0; p < P; ++p)
        pthread_join(p_threads[p], NULL);
    finished = 1;
    for (int k = 0; k < K; ++k)
        pthread_join(k_threads[k], NULL);
}

int main(int argc, char **argv) {
    if (argc < 2) FAILURE_EXIT(2, "./main <config_file>");

    configurate(argv[1]);

    __init__();

    start_threads();

    join_threads();

    __del__();

    return 0;
}