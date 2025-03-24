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

    
}

int read_binary_file(Lamina *lamina, FILE * file){
    if (!file) {
        printf("Error: binary file not found");
        return EXIT_FAILURE;
    }
    unsigned char rows, columns;
    fread(&rows, sizeof(unsigned char), 1, file);
    fread(&columns, sizeof(unsigned char), 1, file);
    printf("Rows: %d\n Columns: %d\n", rows, columns);

    lamina->matrix = (float**)malloc(rows * sizeof(float *));
    for(int i = 0; i< columns; i++){
        lamina->matrix = (float**)malloc(columns * sizeof(float *));
    }
    // TODO: Posiblemente se deban interpretar los numeros de
    // binario a decimal, de momento por facilidad se tomara los
    // numeros tal cual, sin traduccion a decimal
    for(int i = 0; i < rows; i++){
        for(int j = 0; j< columns; j++){
            fread(&lamina->matrix[i][j], sizeof(float), 1, file);
        }
    }
    // TODO: Esto sera eliminado pero de momento para las pruebas
    // se queda.
    fclose(file);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            printf("%.2f ", lamina->matrix[i][j]);
        }
        printf("\n");
    }
    for(int i = 0; i < rows; i++){
        free(lamina->matrix[i]);
    }
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

void print_lamina(){

}

void delete_lamina(){

}