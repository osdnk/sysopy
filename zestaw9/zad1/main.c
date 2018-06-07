#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

char **buffer;
pthread_mutex_t *b_mutex;
pthread_cond_t w_cond;
pthread_cond_t r_cond;
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

    b_mutex = malloc((N + 2) * sizeof(pthread_mutex_t));
    for (int i = 0; i < N + 2; ++i)
        pthread_mutex_init(&b_mutex[i], NULL);

    pthread_cond_init(&w_cond, NULL);
    pthread_cond_init(&r_cond, NULL);

    p_threads = malloc(P * sizeof(pthread_t));
    k_threads = malloc(K * sizeof(pthread_t));
}

void __del__() {
    if (file) fclose(file);

    for (int i = 0; i < N; ++i)
        if (buffer[i]) free(buffer[i]);
    free(buffer);

    for (int j = 0; j < N + 2; ++j)
        pthread_mutex_destroy(&b_mutex[j]);
    free(b_mutex);

    pthread_cond_destroy(&w_cond);
    pthread_cond_destroy(&r_cond);
}

int length_search(int line_length){
    return search == (line_length > L ? 1 : line_length < L ? -1 : 0);
}

void *producer(void *pVoid) {
    int index;
    char line[LINE_MAX];
    while (fgets(line, LINE_MAX, file) != NULL) {
        if(verbose) fprintf(stderr, "Producer[%ld]: taking file line\n", pthread_self());
        pthread_mutex_lock(&b_mutex[N]);

        while (buffer[production_index] != NULL)
            pthread_cond_wait(&w_cond, &b_mutex[N]);

        index = production_index;
        if(verbose) fprintf(stderr, "Producer[%ld]: taking buffer index (%d)\n",  pthread_self(), index);
        production_index = (production_index + 1) % N;


        pthread_mutex_lock(&b_mutex[index]);

        buffer[index] = malloc((strlen(line) + 1) * sizeof(char));
        strcpy(buffer[index], line);
        if(verbose) fprintf(stderr, "Producer[%ld]: line copied to buffer at index (%d)\n",  pthread_self(), index);

        pthread_cond_broadcast(&r_cond);
        pthread_mutex_unlock(&b_mutex[index]);
        pthread_mutex_unlock(&b_mutex[N]);
    }
    if(verbose) fprintf(stderr, "Producer[%ld]: Finished\n", pthread_self());
    return NULL;
}

void *consumer(void *pVoid) {
    char *line;
    int index;
    while (1) {
        pthread_mutex_lock(&b_mutex[N + 1]);

        while (buffer[consumption_index] == NULL) {
            if (finished) {
                pthread_mutex_unlock(&b_mutex[N + 1]);
                if(verbose) fprintf(stderr, "Consumer[%ld]: Finished \n",  pthread_self());
                return NULL;
            }
            pthread_cond_wait(&r_cond, &b_mutex[N + 1]);
        }

        index = consumption_index;
        if(verbose) fprintf(stderr, "Consumer[%ld]: taking buffer index (%d)\n",  pthread_self(), index);
        consumption_index = (consumption_index + 1) % N;

        pthread_mutex_lock(&b_mutex[index]);
        pthread_mutex_unlock(&b_mutex[N + 1]);

        line = buffer[index];
        buffer[index] = NULL;
        if(verbose) fprintf(stderr, "Consumer[%ld]: taking line from buffer at index (%d)\n",  pthread_self(), index);

        pthread_cond_broadcast(&w_cond);
        pthread_mutex_unlock(&b_mutex[index]);

        if(length_search((int) strlen(line))){
            if(verbose) fprintf(stderr, "Consumer[%ld]: found line with length %d %c %d\n",
                                pthread_self(), (int) strlen(line), search == 1 ? '>' : search == -1 ? '<' : '=', L);
            fprintf(stderr, "Consumer[%ld]: Index(%d), %s",  pthread_self(), index, line);
        }
        free(line);
        usleep(10);
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
    pthread_cond_broadcast(&r_cond);
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