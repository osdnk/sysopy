#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <zconf.h>
#include <unistd.h>

#define FAIL(format, ...) {                                            \
    printf(RED_COLOR format RESET_COLOR, ##__VA_ARGS__);                       \
    exit(-1);                                                                  }
#define RED_COLOR "\e[1;31m"
#define RESET_COLOR "\e[0m"


const int buff_size = 512;

typedef struct thread_info {
    int b;
    int e;
    int w;
    int h;
    int filter_size;
    int **picture;
    double **filter;
    int **result;
 } thread_info;

void save_picture(int width, int heigth, int **out_pict_matrix, FILE *fp) {
    char buff[1024];
    fprintf(fp, "P2\n");
    fprintf(fp, "%d %d\n", width, heigth);
    fprintf(fp, "%d\n", 255);
    for (int i = 0; i < heigth; i++) {
        for (int j = 0; j < width; j++) {

            if (j < width - 1) {
                sprintf(buff, "%d ", out_pict_matrix[i][j]);
            } else {
                sprintf(buff, "%d\n", out_pict_matrix[i][j]);
            }
            fputs(buff, fp);
        }
    }
}



double time_diff(clock_t start, clock_t end) {
    return (double) (end - start) / sysconf(_SC_CLK_TCK);
}

void save_time_res(clock_t r_time[2], struct tms tms_time[2], int threads) {
    FILE *fp = fopen("time_res.txt", "a");
    fprintf(fp, "threads: %d\n", threads);
    fprintf(fp, "Real:   %.2lf s   ", time_diff(r_time[0], r_time[1]));
    fprintf(fp, "User:   %.2lf s   ",
            time_diff(tms_time[0].tms_utime, tms_time[1].tms_utime));
    fprintf(fp, "System: %.2lf s\n",
            time_diff(tms_time[0].tms_stime, tms_time[1].tms_stime));
    fprintf(fp, "\n\n");
    fclose(fp);
}


int calc_pixel(int x, int y, int width, int heigth, int c, int **I,
               double **K) {
    double pixel_val = 0;
    for (int j = 0; j < c; j++) {
        int b = (int) round(fmax(0, y - ceil((double) c / 2) + j));
        b = b < heigth ? b : heigth - 1;
        for (int i = 0; i < c; i++) {
            int a = (int) round(fmax(0, x - ceil((double) c / 2) + i));
            a = a < width ? a : width - 1;
            double v = I[b][a] * K[j][i];
            pixel_val += v;
        }
    }
    pixel_val = pixel_val < 0 ? 0 : pixel_val;
    return (int) round(pixel_val);
}


void *single_thread(void *infos) {
    struct thread_info *thread_infos = (struct thread_info *) infos;
    for (int y = 0; y < thread_infos->h; y++)
        for (int x = thread_infos->b; x < thread_infos->e; x++) {
            thread_infos->result[y][x] = calc_pixel(x, y, thread_infos->w, thread_infos->h, thread_infos->filter_size, thread_infos->picture, thread_infos->filter);
        }

    return (void *) 0;

}


int main(int argc, char **argv) {
    if (argc < 5) FAIL("Improper number of arguments\n");
    FILE *file_in = fopen(argv[2], "r+");
    if (file_in == NULL) FAIL("failed in");
    FILE *file_filter = fopen(argv[3], "r+");
    if (file_filter == NULL) FAIL("failed to filter");
    FILE *file_out = fopen(argv[4], "w+");
    if (file_out == NULL) FAIL("failed out");

    char buff[buff_size];

    clock_t r_time[2] = {0, 0};
    struct tms tms_time[2];

    int number_of_threads = (int) strtol(argv[1], NULL, 10);

    fgets(buff, buff_size, file_in);

    fgets(buff, buff_size, file_in);

    char *dimens;

    dimens = strdup(buff);
    int w = (int) strtol(strsep(&dimens, " \t"), NULL, 10);

    int h = (int) strtol(strsep(&dimens, " \t"), NULL, 10);



    int **picture = calloc((size_t) h, sizeof(int *));
    for (int i = 0; i < h; i++) {
        picture[i] = calloc((size_t) w, sizeof(int));
    }

    int **result_picture = calloc((size_t) h, sizeof(int *));
    for (int i = 0; i < h; i++) {
        result_picture[i] = calloc((size_t) w, sizeof(int));
    }

    fgets(buff, buff_size, file_in);

    int iter_column = 0;
    int iter_row = 0;
    while (fgets(buff, buff_size, file_in) != NULL) {
        for (char *word = strtok(buff, " \n\t\r"); word != NULL;
             word = strtok(NULL, " \t\n\r")) {
            picture[iter_row][iter_column] = (int)strtol(word, NULL, 10);
            iter_column++;
            if (iter_column == w) {
                iter_row++;
                iter_column = 0;
            }
        }
    }

    fclose(file_in);


    fgets(buff, buff_size, file_filter);

    int filter_size = (int) strtol(buff, NULL, 10);


    double** filter = calloc((size_t) filter_size, sizeof(double *));
    for (int i = 0; i < filter_size; i++) {
        filter[i] = calloc((size_t) w, sizeof(int));
    }



    for (int i = 0; i < filter_size; i++) {
        filter[i] = calloc((size_t) filter_size, sizeof(double));
        iter_column++;
        if (iter_column == filter_size) {
            iter_row++;
                iter_column = 0;
        }
    }

    iter_column = 0;
    iter_row = 0;
    while (fgets(buff, buff_size, file_filter) != NULL) {
        for (char *word = strtok(buff, " \n\t\r"); word != NULL;
             word = strtok(NULL, " \t\n\r")) {
                filter[iter_row][iter_column] = strtod(word, NULL);
                iter_column++;
                if (iter_column == filter_size) {
                    iter_row++;
                    iter_column = 0;
                }

        }
    }

    pthread_t *thread = calloc((size_t) number_of_threads, sizeof(pthread_t));
    struct thread_info **threads_info = malloc(number_of_threads * sizeof( struct thread_info *));

    r_time[0] = times(&tms_time[0]);

    for (int i = 0; i < number_of_threads; i++) {
        threads_info[i] = malloc(sizeof(struct thread_info));
        threads_info[i]->h = h;
        threads_info[i]->w = w;
        threads_info[i]->filter_size = filter_size;
        threads_info[i]->b = (i * w / number_of_threads);
        threads_info[i]->e = ((i + 1) * w / number_of_threads);
        threads_info[i]->result = result_picture;
        threads_info[i]->picture = picture;
        threads_info[i]->filter = filter;
    }


    for (int i = 0; i < number_of_threads; i++) {
        pthread_create(&thread[i], NULL, single_thread, (void *) threads_info[i]);
    }

    for (int i = 0; i < number_of_threads; i++) {
        pthread_join(thread[i], NULL);
        free(threads_info[i]);
    }

    free(threads_info);

    r_time[1] = times(&tms_time[1]);

    save_picture(w, h, result_picture, file_out);
    save_time_res(r_time, tms_time, number_of_threads);

    for (int i = 0; i < h; i++) {
        free(picture[i]);
    }
    free(picture);

    fclose(file_out);
    for (int i = 0; i < h; i++) {
        free(result_picture[i]);
    }
    free(result_picture);

    fclose(file_filter);
    for (int i = 0; i < filter_size; i++) {
        free(filter[i]);
    }
    free(filter);
}

