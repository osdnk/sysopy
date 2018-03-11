//
// Created by Michał Osadnik on 08/03/2018.
//

#ifndef SYSOPY_LIBRARY_H
#define SYSOPY_LIBRARY_H


struct wrapped_arr {
    int number_of_blocks;
    char ** arr;
    int is_static;
};

struct wrapped_arr *create (int number_of_blocks, int is_static);
void add_block_at_index(struct wrapped_arr * arr, char *block, int index);
void delete_block_at_index(struct wrapped_arr * arr, int index);
void delete_array(struct wrapped_arr * arr);
char *find_closest(struct wrapped_arr * arr, int value);



#endif //SYSOPY_LIBRARY_H
