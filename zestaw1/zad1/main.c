//
// Created by Michał Osadnik on 10/03/2018.
//

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/times.h>
#include <string.h>
#include <zconf.h>
#include "library.h"

char *generate_random_string(int max_size) {
    if (max_size < 1) return NULL;
    char *base = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    size_t dict_len = strlen(base);
    char *res = (char *) malloc((max_size) * sizeof(char));

    for (int i = 0; i < max_size; i++) {
        res[i] = base[rand() % dict_len];
    }

    return res;
}

void fil_array(struct wrapped_arr *arr, int block_size) {
    for (int i = 0; i < arr->number_of_blocks; i++) {
        char *randomString = generate_random_string(block_size);
        add_block_at_index(arr, randomString, i);

    }
}

void add_specific_number_of_blocks(struct wrapped_arr *arr, int number, int start_index, int block_size) {
    for (int i = 0; i < number; i++) {
        char *block = generate_random_string(block_size);
        add_block_at_index(arr, block, i + start_index);
    }
}

void remove_specific_number_of_blocks(struct wrapped_arr *arr, int number, int start_index) {
    for (int i = 0; i < number; i++) {
        delete_block_at_index(arr, i + start_index);
    }
}

void delete_then_add(struct wrapped_arr *arr, int number, int block_size) {
    remove_specific_number_of_blocks(arr, number, 0);
    add_specific_number_of_blocks(arr, number, 0, block_size);
}

void alt_delete_then_add(struct wrapped_arr *arr, int number, int block_size) {
    for (int i = 0; i < number; i++) {
        delete_block_at_index(arr, i);
        add_block_at_index(arr, generate_random_string(block_size), i);
    }
}

double calculate_time(clock_t start, clock_t end) {
    return (double) (end - start) / sysconf(_SC_CLK_TCK);
}

void exec_operation(char *arg, int param, int block_size, struct wrapped_arr *arr) {


    if (strcmp(arg, "change") == 0) {
        delete_then_add(arr, param, block_size);

    } else if (strcmp(arg, "change_alt") == 0) {
        alt_delete_then_add(arr, param, block_size);

    } else if (strcmp(arg, "find") == 0) {
        find_closest(arr, block_size);

    } else if (strcmp(arg, "remove") == 0) {
        delete_array(arr);
    }

}

int main(int argc, char **argv) {
    srand((unsigned int) time(NULL));
    int array_size = (int) strtol(argv[1], NULL, 10);
    int block_size = (int) strtol(argv[2], NULL, 10);

    int is_static;
    if (strcmp(argv[3], "dynamic")) {
        is_static = 0;
    } else if (strcmp(argv[3], "static")) {
        is_static = 1;
    } else {
        printf("Wrong type of allocation!");
        return 0;
    }

    char *first_operation;
    int first_arg = -1;
    char *second_operation;
    int second_arg = -1;
    if (argc >= 5) {
        first_operation = argv[4];
    }
    if (argc >= 6) {
        first_arg = (int) strtol(argv[5], NULL, 10);
    }
    if (argc >= 7) {
        second_operation = argv[6];
    }
    if (argc >= 8) {
        second_arg = (int) strtol(argv[7], NULL, 10);
    }

    struct tms **tms_time = malloc(6 * sizeof(struct tms *));
    for (int i = 0; i < 6; i++) {
        tms_time[i] = (struct tms *) malloc(sizeof(struct tms *));
    }

    printf("   Real      User      System\n");


    struct wrapped_arr *arr;
    arr = create(array_size, is_static);


    times(tms_time[0]);

    fil_array(arr, block_size);

    times(tms_time[1]);


    printf("%s", "create\n");
    printf("%lf   ", calculate_time(tms_time[0]->tms_utime + tms_time[0]->tms_stime,
                                    tms_time[1]->tms_utime + tms_time[1]->tms_stime));
    printf("%lf   ", calculate_time(tms_time[0]->tms_utime, tms_time[1]->tms_utime));

    printf("%lf ", calculate_time(tms_time[0]->tms_stime, tms_time[1]->tms_stime));
    printf("\n");

    if (argc >= 5) {

        times(tms_time[2]);
        exec_operation(first_operation, first_arg, block_size, arr);
        times(tms_time[3]);


        printf("%s %s", first_operation, " \n");
        printf("%lf   ", calculate_time(tms_time[2]->tms_utime + tms_time[2]->tms_stime,
                                        tms_time[3]->tms_utime + tms_time[3]->tms_stime));
        printf("%lf   ", calculate_time(tms_time[2]->tms_utime, tms_time[3]->tms_utime));
        printf("%lf ", calculate_time(tms_time[2]->tms_stime, tms_time[3]->tms_stime));
        printf("\n");
    }

    if (argc >= 7) {

        times(tms_time[4]);

        exec_operation(second_operation, 300000, block_size, arr);
        times(tms_time[5]);

        printf("%s %s", second_operation, " \n");
        printf("%lf   ", calculate_time(tms_time[4]->tms_utime + tms_time[4]->tms_stime,
                                        tms_time[5]->tms_utime + tms_time[5]->tms_stime));
        printf("%lf   ", calculate_time(tms_time[4]->tms_utime, tms_time[5]->tms_utime));
        printf("%lf ", calculate_time(tms_time[4]->tms_stime, tms_time[5]->tms_stime));
        printf("\n");
    }


    return 0;
}
