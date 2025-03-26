#include "lamina.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

int lamina_constructor(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Invalid entrance: you need to specify a .txt and the amounth of"
            "threads you want to use\n");
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Error: invalid file\n");
        return EXIT_FAILURE;
    }
    Lamina * lamina = (Lamina *)malloc(sizeof(Lamina));
    if (!lamina) {
        printf("Error: could not allocate memory for Lamina\n");
        return EXIT_FAILURE;
    }
    char *bin_file = (char *)malloc(256 * sizeof(char));
    char temp[256];
    char line[256];
    //TODO: Este while es feisimo, lo se, la idea es cambiarlo a futuro
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%s %" PRIu64 "%f %f %lf", temp, &lamina->time, &lamina->conductivity,
              &lamina->height, &lamina->epsilon) != 5) {
            printf("Error: The file of configuration does not have the spected parameters");
            return EXIT_FAILURE;
        }
        snprintf(bin_file, 256, "../jobs/%s", temp);
        reading_parameters(lamina, bin_file);
        lamina->file = fopen(bin_file, "rb");
        if (!lamina->file) {
            printf("Error: Could not open the binary file");
            return EXIT_FAILURE;
        }
        printf("Creating lamina...\n");
        if (create_lamina(lamina) == EXIT_FAILURE) {
            printf("Error: Could not create the lamina\n");
            return EXIT_FAILURE;
        }
    }
    free(bin_file);
    return EXIT_SUCCESS;
}

// TODO: Funcion que sera borrada pues no hace nada, la tengo de momento solo
// para imprimir
void reading_parameters(Lamina *lamina, char* bin_file){
    // TODO: Esta impresion es temporal, es solo para ver que todo va bien
    char temp[256];
    // TODO: verificar que sean valores validos, tipo height no puede ser 0
    printf("Binary file: %s\n", bin_file);
    printf("Time: %"PRIu64 "\n", lamina->time);
    printf("Conductivity: %.f\n", lamina->conductivity);
    printf("Height: %.f\n", lamina->height);
    printf("Epsilon: %.15f\n", lamina->epsilon);
    // TODO: puede llegar a caerse si bin_file se limpia antes de retornar
}

int create_lamina(Lamina *lamina){
    lamina->rows = 0;
    lamina->columns = 0;
    if (fread(&lamina->rows, sizeof(uint64_t), 1, lamina->file) != 1) {
        printf("Error: Failed to read rows from file\n");
        fclose(lamina->file);
        return EXIT_FAILURE;
    }
    if (fread(&lamina->columns, sizeof(uint64_t), 1, lamina->file) != 1) {
        printf("Error: Failed to read columns from file\n");
        fclose(lamina->file);
        return EXIT_FAILURE;
    }
    printf("Rows: %ld \nColumns: %ld\n", lamina->rows, lamina->columns);
    lamina->temperatures = (double **)malloc(lamina->rows * sizeof(double *));
    lamina->next_temperatures = (double **)malloc(lamina->rows * sizeof(double *));
    // TODO: Falta agregar if en caso de no poder reservar memoria
    for (uint64_t i = 0; i < lamina->rows; i++) {
        lamina->temperatures[i] = (double *)malloc(lamina->columns * sizeof(double));
        lamina->next_temperatures[i] = (double *)malloc(lamina->columns * sizeof(double));
    }
    // TODO: Falta agregar if en caso de no poder reservar memoria
    for (uint64_t i = 0; i < lamina->rows; i++) {
        for (uint64_t j = 0; j < lamina->columns; j++) {
            fread(&lamina->temperatures[i][j], sizeof(double), 1, lamina->file);
        }
    }
    // TODO: Impresion temporal, es solo para ver que todo sirva hasta aca
    print_lamina(lamina);
    update_lamina(lamina);

    return EXIT_SUCCESS;
}

void update_lamina(Lamina * lamina){
    uint64_t unstable_cells;
    do {
        unstable_cells = 0;
        // vamos aumentando el tiempo de la simulacion a cada estado procesado
        lamina->time = lamina->time * lamina->k;
        for (int i = 0; i < lamina->rows; i++) {
            for (int j = 0; j < lamina->columns; j++) {
                if (i == 0 || i == lamina->rows - 1 || j == 0 || j == lamina->columns - 1) {
                    // Se ignoran los bordes porque son constantes
                    continue;
                }
                lamina->next_temperatures[i][j] = 0;
                update_cell(lamina, i, j);
                if (fabs(lamina->next_temperatures[i][j] - lamina->temperatures[i][j]) > lamina->epsilon) {
                    unstable_cells++;
                }
            }
        }
        // TODO: Temporal, es solo para ver como se actualizo
        printf("//////////////////////////////////////////////////\n");
        printf("//////////////////////////////////////////////////\n");
        print_lamina(lamina);
        printf("//////////////////////////////////////////////////\n");
        printf("//////////////////////////////////////////////////\n");
        double **temp = lamina->temperatures;
        lamina->temperatures = lamina->next_temperatures;
        lamina->next_temperatures = temp;
        lamina->k++;
    } while (unstable_cells > 0);
    // finish_simulation(lamina);
}

void update_cell(Lamina * lamina, uint64_t row, uint64_t column){
    double up, down, left, right = 0;
    // condicionales para asegurar que nunca se toman valores que no pertenecen
    // a la matriz temperaturas.
    if (row + 1 < lamina->rows) {
        up = lamina->temperatures[row + 1][column];
    }
    if (row - 1 >= 0) {
        down = lamina->temperatures[row - 1][column];
    }
    if (column + 1 < lamina->columns) {
        right = lamina->temperatures[row][column + 1];
    }
    if (column - 1 >= 0) {
        left = lamina->temperatures[row][column - 1];
    }
    // Esta es la formula indicada en el enunciado de tarea01 para
    // obtener la temperatura de la celda actual pero en el siguiente estado
    lamina->next_temperatures[row][column] = lamina->temperatures[row][column] +
    ((lamina->time * lamina->conductivity) /
    (lamina->height * lamina->height)) *
    (up + down + right + left - (4 * lamina->temperatures[row][column]));
}

void finish_simulation(Lamina * lamina){

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