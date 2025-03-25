#include "lamina.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void lamina_constructor(const char* filename_, uint64_t threads_num) {
    Lamina * lamina = (Lamina *)malloc(sizeof(Lamina));
    if (!lamina) {
        printf("Error: could not allocate memory for Lamina\n");
        return;
    }
    lamina->binary_file = fopen(filename_, "rb+");
    if(read_binary_file(lamina, lamina->binary_file) != EXIT_SUCCESS) {
        return;
    }

    printf("Nose\n");
}

int read_binary_file(Lamina *lamina, FILE * file){
    if (!file) {
        printf("Error: binary file not found\n");
        return EXIT_FAILURE;
    }
    //inicializamos los valores para limpiar posible basura
    lamina->rows = 0;
    lamina->columns = 0;
    if (fread(&lamina->rows, sizeof(uint64_t), 1, file) != 1) {
        printf("Error: Failed to read rows from file\n");
        fclose(file);
        return EXIT_FAILURE;
    }
    if (fread(&lamina->columns, sizeof(uint64_t), 1, file) != 1) {
        printf("Error: Failed to read columns from file\n");
        fclose(file);
        return EXIT_FAILURE;
    }
    printf("Rows: %ld \nColumns: %ld\n", lamina->rows, lamina->columns);
    // Asignar memoria para la->temperatures
    lamina->temperatures = (double **)malloc(lamina->rows * sizeof(double *));
    for (uint64_t i = 0; i < lamina->rows; i++) {
        lamina->temperatures[i] = (double *)malloc(lamina->columns * sizeof(double));
    }

    // Leer los valores de la->temperatures
    for (uint64_t i = 0; i < lamina->rows; i++) {
        for (uint64_t j = 0; j < lamina->columns; j++) {
            fread(&lamina->temperatures[i][j], sizeof(double), 1, file);
        }
    }
    print_lamina(lamina);
    return EXIT_SUCCESS;
}

void create_lamina(uint64_t rows, uint64_t columns){

}

void update_lamina(){

}

void update_cell(){

}

void finish_simulation(){

}

void print_lamina(Lamina* lamina){
    printf("Matriz de temperaturas:\n");
    for (uint64_t i = 0; i < lamina->rows; i++) {
        for (uint64_t j = 0; j < lamina->columns; j++) {
            printf("%8.2f ", lamina->temperatures[i][j]); // Imprime con 2 decimales y alineado
        }
        printf("\n"); // Salto de línea por fila
    }
}

void delete_lamina(){

}