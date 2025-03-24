#ifndef LAMINA_H
#define LAMINA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    const uint64_t rows;
    const uint64_t columns;
    uint64_t k;
    const float conductivity;
    const float cell_height;
    const float epsilon;
    float ** temperatures;
    float ** matrix;
    FILE *binary_file;
} Lamina;


void lamina_constructor(const char* filename_, uint64_t threads_num);
int read_binary_file(Lamina * lamina, FILE * binary_file_);
void create_lamina(uint64_t rows, uint64_t columns);
void update_lamina();
void update_cell();
void finish_simulation();
void print_lamina();
void delete_lamina();

#endif