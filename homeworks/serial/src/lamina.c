// Copyright 2025 Anthony Sanchez
#include "lamina.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <time.h>

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
    char temp[256];
    char line[256];
    strcpy(lamina->rutaBase, argv[1]);
    char * lastSLash = strrchr(lamina->rutaBase, '/');
    if (!lastSLash) {
        fprintf(stderr, "Error: invalid route %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    *(lastSLash + 1) = '\0';
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
        snprintf(lamina->bin_file_name, 260, "%s%s", lamina->rutaBase, temp);
        if (reading_parameters(lamina, lamina->bin_file_name) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        lamina->file = fopen(lamina->bin_file_name, "rb");
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
    lamina->next_temperatures = (double **)malloc(lamina->rows * sizeof(double*));
    if (!lamina->temperatures || !lamina->next_temperatures) {
        fprintf(stderr, "Error: No se pudo reservar memoria para las filas"
            " de las matrices.\n");
        free(lamina->temperatures);
        free(lamina->next_temperatures);
        return EXIT_FAILURE;
    }

    // Reservar memoria para cada fila
    for (uint64_t i = 0; i < lamina->rows; i++) {
        lamina->temperatures[i] = (double *)malloc(lamina->columns * sizeof(double));
        lamina->next_temperatures[i] = (double *)malloc(lamina->columns * sizeof(double));

        // Comprobar si la reserva de memoria falló para alguna fila
        if (!lamina->temperatures[i] || !lamina->next_temperatures[i]) {
            fprintf(stderr, "Error: No se pudo reservar memoria para la fila %" PRIu64 " de temperatures.\n", i);
            free(lamina->temperatures);
            free(lamina->next_temperatures);
            return EXIT_FAILURE;
        }
    }
    // Bucle para leer los valores de la matriz
    for (uint64_t i = 0; i < lamina->rows; i++) {
        for (uint64_t j = 0; j < lamina->columns; j++) {
            fread(&lamina->temperatures[i][j], sizeof(double), 1, lamina->file);
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
    finish_simulation(lamina);
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

int finish_simulation(Lamina * lamina) {
    char report_file_name[256];
    strncpy(report_file_name, lamina->bin_file_name, sizeof(report_file_name) - 1);
    report_file_name[sizeof(report_file_name) - 1] = '\0';

    printf("Ruta original copiada: %s\n", report_file_name);
    char *dot = strrchr(report_file_name, '.');
    if (dot == NULL) {
        printf("Error: No se encontró un punto en la ruta del archivo.\n");
        return EXIT_FAILURE;
    }
    printf("Punto encontrado en: %s\n", dot);
    snprintf(dot, 5, ".tsv");
    printf("Nuevo nombre de archivo: %s\n", report_file_name);
    FILE *report_file = fopen(report_file_name, "a");
    if (!report_file) {
        printf("Error: No se pudo abrir el archivo %s\n", report_file_name);
        return EXIT_FAILURE;
    }
    uint64_t total_k = lamina->k / lamina->time;
    char text[256];
    snprintf(text, sizeof(text), "%" PRIu64, total_k);
    char *final_text = format_time(lamina->time, text, sizeof(text));
    printf("A VER: %s y la ruta es %s\n", final_text, report_file_name);
    fprintf(report_file, "%s", final_text);
    fclose(report_file);
    return EXIT_SUCCESS;
}

void print_lamina(Lamina* lamina) {
    printf("Matriz de temperaturas:\n");
    for (uint64_t i = 0; i < lamina->rows; i++) {
        for (uint64_t j = 0; j < lamina->columns; j++) {
            printf("[%.17lf] ", lamina->temperatures[i][j]);
        }
        printf("\n");
    }
}

void delete_lamina() {
}

char* format_time(const time_t seconds, char* text, const size_t capacity) {
    struct tm* gmt = gmtime(&seconds);
    snprintf(text, capacity, "%04d/%02d/%02d\t%02d:%02d:%02d", gmt->tm_year - 70,
        gmt->tm_mon, gmt->tm_mday - 1, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
    return text;
  }