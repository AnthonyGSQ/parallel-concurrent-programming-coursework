#ifndef LAMINA_H
#define LAMINA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint64_t rows;
    uint64_t columns;
    uint64_t k;
    uint64_t time;
    double conductivity;
    double height;
    double epsilon;
    double ** temperatures;
    double ** next_temperatures;
    FILE *file;
} Lamina;

int lamina_constructor(int argc, char *argv[]);
int validate_entrance(int argc, char *argv[]);
void reading_parameters(Lamina *lamina, char* filename);
int create_lamina(Lamina *lamina);
void update_lamina(Lamina * lamina);
void update_cell(Lamina * lamina, uint64_t row, uint64_t column);
void finish_simulation(Lamina * lamina);
void print_lamina(Lamina * lamina);
void delete_lamina();

#endif