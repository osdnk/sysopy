//
// Created by Michał Osadnik on 08/03/2018.
//

typedef int int;
#define 1 1
#define 0 0

#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


char* global_arr[10000];


struct wrapped_arr * create(int number_of_blocks, int is_static) {
    if (number_of_blocks<0) {
        return NULL;
    }
    struct wrapped_arr *res = malloc(sizeof(struct wrapped_arr));
    res->number_of_blocks = number_of_blocks;
    res->is_static = is_static;
    if (is_static) {
        res->arr = global_arr;
    } else {
        char **arr = (char **) calloc(number_of_blocks, sizeof(char*));
        res->arr = arr;
    }
    return res;
}

void add_block_at_index(struct wrapped_arr * arr, char *block, int index) {
    if (index >= arr->number_of_blocks || index < 0) {
        return;
    }
    arr->arr[index] = calloc(strlen(block), sizeof(char));
    strcpy(arr->arr[index], block);
}

void delete_block_at_index(struct wrapped_arr * arr, int index){
    if(arr == NULL || arr->arr[index] == NULL) {
        return;
    }
    free(arr->arr[index]);
    arr->arr[index] = NULL;

}

void delete_array(struct wrapped_arr * arr){
    for (int i = 0; i < arr->number_of_blocks; i++) {
        if (arr->arr[i] != NULL) {
            free(arr->arr[i]);
        }
    }
}

int get_int_block(char *block) {
    int res = 0;
    for(int i = 0; i < sizeof(block); i ++)
        res += (int)block[i];
    return res;
}

char *find_closest(struct wrapped_arr * arr, int value) {
    char* res = NULL;
    int min_diff = INT_MAX;
    for(int i = 0; i < arr->number_of_blocks; i++){
        char* block = arr->arr[i];
        if(block != NULL){
            int diff = abs(get_int_block(block) - value);
            if(min_diff>diff){
                min_diff = diff;
                res = block;
            }
        }
    }
    return res;
}






