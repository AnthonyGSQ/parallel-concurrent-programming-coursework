// Copyright 2025 Anthony Sanchez
#include "lamina.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

int lamina_constructor(int argc, char *argv[]) {
    // En caso de recibir menos parametros a los esperados, retorna EXIT_FAILURE
    if (argc < 3) {
        printf("Invalid entrance: you need to specify a .txt and the amounth of"
            "threads you want to use\n");
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "r");
    // En caso de no poder abrir el archivo, retorna EXIT_FAILURE
    if (!file) {
        printf("Error: invalid file\n");
        return EXIT_FAILURE;
    }
    Lamina * lamina = (Lamina *)malloc(sizeof(Lamina));
    // En caso de no poder reservar memoria para lamina, retorna EXIT_FAILURE
    if (!lamina) {
        printf("Error: could not allocate memory for Lamina\n");
        return EXIT_FAILURE;
    }
    char *bin_file = (char *)malloc(256 * sizeof(char));
    char temp[256];
    char line[256];
    // TODO(AnthonyGSQ): Este while es feisimo, lo se, la idea es cambiarlo a
    // futuro
    while (fgets(line, sizeof(line), file)) {
        // Si la cantidad de parametros recibidos no es 5
        // se retorna EXIT_FAILURE
        if (sscanf(line, "%s %" PRIu64 "%lf %lf %lf", temp, &lamina->time,
              &lamina->conductivity,
                &lamina->height, &lamina->epsilon) != 5) {
            printf("Error: The file of configuration does not have the spected"
                  "parameters");
            return EXIT_FAILURE;
        }
        // TODO(AnthonyGSQ): Quiza se deba cambiar la logica de la ruta
        snprintf(bin_file, sizeof(bin_file), "../jobs/%s", temp);
        if (reading_parameters(lamina, bin_file) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
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

int reading_parameters(Lamina *lamina, char* bin_file) {
    printf("Binary file: %s\n", bin_file);
    printf("Time: %"PRIu64 "\n", lamina->time);
    // Ifs que verifican que los valores recibidos del txt sean validos, si no
    // lo son, retornan EXIT_FAILURE
    if (lamina->time == 0) {
        printf("Error: Invalid time: %" PRIu64 "\n", lamina->time);
        return EXIT_FAILURE;
    }
    printf("Conductivity: %.f\n", lamina->conductivity);
    if (lamina->conductivity <= 0) {
        printf("Error: Invalid conductivity: %lf\n", lamina->conductivity);
        return EXIT_FAILURE;
    }
    printf("Height: %.f\n", lamina->height);
    if (lamina->height <= 0) {
        printf("Error: Invalid height: %lf\n", lamina->height);
        return EXIT_FAILURE;
    }
    printf("Epsilon: %.15f\n", lamina->epsilon);
    if (lamina->epsilon <= 0) {
        printf("Error: Invalid parameter epsilon: %lf\n", lamina->epsilon);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int create_lamina(Lamina *lamina) {
    // Inicializamos rows y columns en 0 para evitar posibles valorea basura
    lamina->rows = 0;
    lamina->columns = 0;
    // Se lee el primer bloque de bytes donde se encuentra el numero de filas
    if (fread(&lamina->rows, sizeof(uint64_t), 1, lamina->file) != 1) {
        printf("Error: Failed to read rows from file\n");
        fclose(lamina->file);
        return EXIT_FAILURE;
    }
    // Si el numero de filas es invalido, se retorna EXIT_FAILURE
    if (lamina->rows <= 0) {
        printf("Error: Invalid number of rows");
        return EXIT_FAILURE;
    }
    // Se lee el segundo bloque de bytes donde se encuentra el numero de
    // columnas
    if (fread(&lamina->columns, sizeof(uint64_t), 1, lamina->file) != 1) {
        printf("Error: Failed to read columns from file\n");
        fclose(lamina->file);
        return EXIT_FAILURE;
    }
    // Si el numero de columnas es invalido, se retorna EXIT_FAILURE
    if (lamina->columns <= 0) {
        printf("Error: Invalid number of columns");
        return EXIT_FAILURE;
    }
    printf("Rows: %ld \nColumns: %ld\n", lamina->rows, lamina->columns);
    // Reservamos memoria para las filas de las dos matrices de la lamina
    lamina->temperatures = (double **)malloc(lamina->rows * sizeof(double *));
    if (lamina->temperatures == NULL) {
        fprintf(stderr, "Error: No se pudo reservar memoria para"
            "temperatures.\n");
        return EXIT_FAILURE;
    }
    lamina->next_temperatures = (double **)malloc(lamina->rows *
        sizeof(double *));
    if (lamina->next_temperatures == NULL) {
        fprintf(stderr, "Error: No se pudo reservar memoria para"
            "next_temperatures.\n");
        free(lamina->temperatures);
        return EXIT_FAILURE;
    }
    // Bucle para reservar memoria para las columnas de todas las filas de las
    // dos matrices de la lamina, en caso de no poder se libera la memoria
    // reservada en las columnas antes del fallo
    for (uint64_t i = 0; i < lamina->rows; i++) {
        lamina->temperatures[i] = (double *)malloc(lamina->columns *
            sizeof(double));
        if (lamina->temperatures[i] == NULL) {
            fprintf(stderr, "Error: No se pudo reservar memoria para"
                " la fila %llu de temperatures.\n", i);
            // Liberar memoria previamente reservada
            for (uint64_t j = 0; j < i; j++) {
                free(lamina->temperatures[j]);
                free(lamina->next_temperatures[j]);
            }
            free(lamina->temperatures);
            free(lamina->next_temperatures);
            exit(1);
        }
        lamina->next_temperatures[i] = (double *)malloc(lamina->columns *
            sizeof(double));
        if (lamina->next_temperatures[i] == NULL) {
            fprintf(stderr, "Error: No se pudo reservar memoria para la"
                "fila %llu de next_temperatures.\n", i);
            // Liberar memoria previamente reservada
            for (uint64_t j = 0; j < i; j++) {
                free(lamina->temperatures[j]);
                free(lamina->next_temperatures[j]);
            }
            free(lamina->temperatures);
            free(lamina->next_temperatures);
            exit(1);
        }
    }
    // TODO(AnthonyGSQ): Impresion temporal, es solo para ver que todo sirva
    // hasta aca
    print_lamina(lamina);
    update_lamina(lamina);

    return EXIT_SUCCESS;
}

// TODO(AnthonyGSQ): Falta agregar logica de conductivity
void update_lamina(Lamina * lamina) {
    double diff;
    double scale = 1e15;
    uint64_t unstable_cells = 1;
    double** temp;
    while (unstable_cells > 0) {
        // El valor de unstable_cells se resetea despues de procesar una vez
        // la lamina entera, asi aseguramos que el bucle no termine hasta
        // no encontrar ni una sola celda inestable
        unstable_cells = 0;
        // Para evitar calculos incorrectos, se resetea la
        //  matriz next_temperatures
        for (uint64_t i = 0; i < lamina->rows; i++) {
            for (uint64_t j = 0; j < lamina->columns; j++) {
                lamina->next_temperatures[i][j] = 0;
            }
        }

        for (uint64_t i = 0; i < lamina->rows; i++) {
            for (uint64_t j = 0; j < lamina->columns; j++) {
                // if para ignorar los bordes de la matriz
                if (i == 0 || i == lamina->rows - 1 ||
                      j == 0 || j == lamina->columns -1) {
                        continue;
                }
                update_cell(lamina, i, j);
                diff = fabs(lamina->next_temperatures[i][j] -
                    lamina->temperatures[i][j]);
                // SI la celda actual no esta estable, unstable_cells aumenta
                // su valor, asegurando que la lamina sea procesada un estado
                // mas
                if (diff > lamina->epsilon) {
                    unstable_cells++;
                } else {
                    printf("Diferencia: %.17lf\n", diff);
                }
            }
        }
        // EL estado k+1 pasa a ser el estado k y el estado k pasa a ser el
        // estado k+1 para ser reseteada enteramente.
        temp = lamina->temperatures;
        lamina->temperatures = lamina->next_temperatures;
        lamina->next_temperatures = temp;
        lamina->k += lamina->time;
    }
    printf("///////////////////////////////////////////\n");
    print_lamina(lamina);
    printf("Lamina estabilizada\n");
    printf("///////////////////////////////////////////\n");
}

void update_cell(Lamina * lamina, uint64_t row, uint64_t column) {
    double up = 0, down = 0, left = 0, right = 0;
    // Esta es la formula indicada en el enunciado de tarea01 para
    // obtener la temperatura de la celda actual pero en el siguiente estado
    lamina->next_temperatures[row][column] = lamina->temperatures[row][column] +
    ((lamina->k * lamina->conductivity) /
    (lamina->height * lamina->height)) *
    (up + down + right + left - (4 * lamina->temperatures[row][column]));
}

void finish_simulation(Lamina * lamina) {
}

void print_lamina(Lamina* lamina) {
    printf("Matriz de temperaturas:\n");
    for (uint64_t i = 0; i < lamina->rows; i++) {
        for (uint64_t j = 0; j < lamina->columns; j++) {
            printf("%.17lf ", lamina->temperatures[i][j]);
        }
        printf("\n");
    }
}

void delete_lamina() {
}
