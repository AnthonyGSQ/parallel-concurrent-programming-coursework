// Copyright 2025 Anthony Sanchez
#include "lamina.h"
#include "FileManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <time.h>
/***
 * @file lamina.c
 * @brief Implementacion de la funcion lamina.c
 * @author Anthony Sanchez
 * @date 2025-03-29
 */

int lamina_constructor(int argc, char *argv[]) {
    int return_value = 0;
    // En caso de recibir menos parametros a los esperados, retorna EXIT_FAILURE
    if (argc < 2) {
        error_manager(NULL, NULL, "Invalid entrance: you need to specify a .txt and"
            "the amounth of threads you want to use");
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Error: could not open %s file", argv[1]);
        return EXIT_FAILURE;
    }
    char line[256] = "";
    // while para leer todos los reglones del txt
    while (fgets(line, sizeof(line), file)) {
        Lamina *lamina = (Lamina *)calloc(1, sizeof(Lamina));
        shared_file_data * fileobj = (shared_file_data*)calloc(1,sizeof(shared_file_data));
        if (!lamina) {
            error_manager(lamina, fileobj, "Error: could not allocate memory for"
                "Lamina");
            return EXIT_FAILURE;
        }
        snprintf(fileobj->base_route, sizeof(fileobj->base_route), "%s", argv[1]);

        char *lastSLash = strrchr(fileobj->base_route, '/');
        if (!lastSLash) {
            fprintf(stderr, "Error: invalid route %s\n", argv[1]);
            free(lamina);
            return EXIT_FAILURE;
        }
        *(lastSLash + 1) = '\0';
        if (strcmp(line, "") == 0) {
            error_manager(lamina, fileobj, "Error: could not read a line from the .txt"
                "file");
                return EXIT_FAILURE;
        }
        // Si la cantidad de parametros recibidos no es 5
        // se retorna EXIT_FAILURE
        return_value  = reading_parameters(lamina, fileobj, fileobj->binary_file_name,
            line);
        if (return_value == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        fileobj->file = fopen(fileobj->binary_file_name, "rb");
        printf("Creating lamina...\n");
        // si algo fallo en la creacion de la lamina o en la estabilizacion
        // de la misma, se retorna EXIT_FAILURE
        return_value = create_lamina(lamina, fileobj);
        if (return_value == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        delete_lamina(lamina, fileobj);
    }
    return EXIT_SUCCESS;
}
int reading_parameters(Lamina *lamina, shared_file_data* fileobj, char* bin_file, char* line) {
    char temp[256] = "";
    // si la linea del txt no cuenta con los 5 parametros esperados
    // retorna EXIT_FAILURE
    if (sscanf(line, "%s %" PRIu64 "%lf %lf %lf", temp, &lamina->time,
        &lamina->conductivity,
          &lamina->height, &lamina->epsilon) != 5) {
            error_manager(lamina, fileobj, "Error: The file of configuration does not"
                "have the spected parameters");
                return EXIT_FAILURE;
    }
    // se agrega la ruta al binario para poder leer el archivo
  snprintf(fileobj->binary_file_name,
      sizeof(fileobj->binary_file_name) + sizeof(temp), "%s%s",
      fileobj->base_route, temp);
    // Ifs que verifican que los valores recibidos del txt sean validos, si no
    // lo son, retornan EXIT_FAILURE
    if (lamina->time == 0) {
        error_manager(lamina, fileobj, "Invalid time");
        return EXIT_FAILURE;
    }
    printf("Conductivity: %.f\n", lamina->conductivity);
    if (lamina->conductivity <= 0) {
        error_manager(lamina, fileobj, "Invalid conductivity");
        return EXIT_FAILURE;
    }
    printf("Height: %.f\n", lamina->height);
    if (lamina->height <= 0) {
        error_manager(lamina, fileobj, "Invalid height");
        return EXIT_FAILURE;
    }
    printf("Epsilon: %.15f\n", lamina->epsilon);
    if (lamina->epsilon <= 0) {
        error_manager(lamina, fileobj, "Invalid epsilon");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
int create_lamina(Lamina *lamina, shared_file_data* fileobj) {
    // Inicializamos rows y columns en 0 para evitar posibles valorea basura
    lamina->rows = 0;
    lamina->columns = 0;
    int return_value = 0;
    // Se lee el primer bloque de bytes donde se encuentra el numero de filas
    if (fread(&lamina->rows, sizeof(uint64_t), 1, fileobj->file) != 1) {
        error_manager(lamina, fileobj, "Error: Failed to read rows from file");
        return EXIT_FAILURE;
    }
    // Si el numero de filas es invalido, se retorna EXIT_FAILURE
    if (lamina->rows <= 0) {
        error_manager(lamina, fileobj, "Error: Invalid number of rows");
        return EXIT_FAILURE;
    }
    // Se lee el segundo bloque de bytes donde se encuentra el numero de
    // columnas
    if (fread(&lamina->columns, sizeof(uint64_t), 1, fileobj->file) != 1) {
        error_manager(lamina, fileobj, "Error: Failed to read columns from file");
        return EXIT_FAILURE;
    }
    // Si el numero de columnas es invalido, se retorna EXIT_FAILURE
    if (lamina->columns <= 0) {
        error_manager(lamina, fileobj, "Error: Invalid number of columns");
        return EXIT_FAILURE;
    }
    // Reservamos memoria para las filas de las dos matrices de la lamina
    lamina->temperatures = (double **)malloc(lamina->rows * sizeof(double *));
    lamina->next_temperatures = (double **)malloc(lamina->rows *
        sizeof(double*));
    if (!lamina->temperatures || !lamina->next_temperatures) {
        error_manager(lamina, fileobj, "Error: No se pudo reservar memoria para las"
            "filas de las matrices.");
            return EXIT_FAILURE;
    }

    // Reservar memoria para cada fila
    for (size_t i = 0; i < lamina->rows; i++) {
        lamina->temperatures[i] = (double *)calloc(lamina->columns,
            sizeof(double));
        lamina->next_temperatures[i] = (double *)calloc(lamina->columns,
            sizeof(double));

        /*
        lamina->temperatures[i] = (double *)malloc(lamina->columns *
            sizeof(double));
        lamina->next_temperatures[i] = (double *)malloc(lamina->columns *
            sizeof(double));
            
        */
        // Comprobar si la reserva de memoria falló para alguna fila
        if (!lamina->temperatures[i] || !lamina->next_temperatures[i]) {
            error_manager(lamina, fileobj, "Error: No se pudo reservar memoria para las"
                "filas de las matrices.");
                return EXIT_FAILURE;
        }
    }
    return_value = fillMatriz(lamina, fileobj);
    if(return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int fillMatriz(Lamina *lamina, shared_file_data* fileobj) {
    int return_value = 0;
    // Bucle para leer los valores de la matriz
    for (size_t i = 0; i < lamina->rows; i++) {
        for (size_t j = 0; j < lamina->columns; j++) {
            if (fread(&lamina->temperatures[i][j], sizeof(double), 1,
            fileobj->file) != 1) {
                error_manager(lamina, fileobj, "Error: Failed to read temperature values"
                    "from file");
                return EXIT_FAILURE;
            }
        }
    }
    // TODO(AnthonyGSQ): Impresion temporal, es solo para ver que todo sirva
    // hasta aca
    print_lamina(lamina);
    return_value = update_lamina(lamina, fileobj);
    if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
int update_lamina(Lamina * lamina, shared_file_data *fileobj) {
    int return_value = 0;
    // variable donde se guarda la diferencia de la celda actual con su futuro
    // estado para compararla con el epsilon y saber si dicha celda esta estable
    double diff = 0;
    // variable para contar cuantas celdas no estan estables, con solo una no
    // estable, el while se vuelve a ejecutar
    size_t unstable_cells = 1;
    double** temp = NULL;
    while (unstable_cells > 0) {
        // El valor de unstable_cells se resetea despues de procesar una vez
        // la lamina entera, asi aseguramos que el bucle no termine hasta
        // no encontrar ni una sola celda inestable
        unstable_cells = 0;
        // Para evitar calculos incorrectos, se resetea la
        //  matriz next_temperatures
        for (size_t i = 0; i < lamina->rows; i++) {
            for (size_t j = 0; j < lamina->columns; j++) {
                if (i == 0 || i == lamina->rows - 1 ||
                        j == 0 || j == lamina->columns -1) {
                    continue;
                }
                lamina->next_temperatures[i][j] = 0;
            }
        }

        for (size_t i = 0; i < lamina->rows; i++) {
            for (size_t j = 0; j < lamina->columns; j++) {
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
    printf("Lamina estabilizada: \n");
    print_lamina(lamina);
    return_value = finish_simulation(lamina, fileobj);
    if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
void update_cell(Lamina * lamina, uint64_t row, uint64_t column) {
    // aseguramos que no se procesara una celda invalida
    if (row > 0 && row < lamina->rows - 1 && column > 0 &&
        column < lamina->columns - 1) {
        double up = lamina->temperatures[row - 1][column];
        double down = lamina->temperatures[row + 1][column];
        double left = lamina->temperatures[row][column - 1];
        double right = lamina->temperatures[row][column + 1];
        // formula para actualizar la temperatura de la celda
        lamina->next_temperatures[row][column] =
        lamina->temperatures[row][column] +
        ((lamina->k * lamina->conductivity) /
        (lamina->height * lamina->height)) *
        (up + down + right + left - (4 * lamina->temperatures[row][column]));
    }
}
int finish_simulation(Lamina * lamina, shared_file_data *fileobj) {
    int return_value = report_file(lamina, fileobj);
    if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
int report_file(Lamina *lamina, shared_file_data *fileobj) {
    char text[260] = "";
    char temp[260] = "";
    char file_name[260] = "";
    char alternative_route[260] = "";
    snprintf(file_name, sizeof(file_name), "%s", fileobj->binary_file_name);
    char *lastSLash = strrchr(file_name, '/') + 1;
    char *dot = strrchr(file_name, '.');
    *dot = '\0';
    snprintf(alternative_route, sizeof(alternative_route), "%s%s%s",
    fileobj->base_route, lastSLash, ".tsv");
    snprintf(temp, sizeof(temp), "%s%s%s%s", fileobj->base_route,
    "tsv/", lastSLash, ".tsv");
    // si no se encuentra la carpeta tsv, crea el archivo tsv en la misma
    // carpeta donde se encuentra el binario leido
    fileobj->report_file = fopen(temp, "w+");
    if (!fileobj->report_file) {
        printf("Error: could not open tsv file\n");
        printf("Creating new tsv file\n");
        fileobj->report_file = fopen(alternative_route, "w+");
    }
    snprintf(text, sizeof(temp), "%s\t%" PRIu64 "\t%g\t%g\t%g\t", temp,
    lamina->time, lamina->conductivity, lamina->height, lamina->epsilon);
    format_time(lamina->k, text, sizeof(text));
    fprintf(fileobj->report_file, "%s", text);
    return EXIT_SUCCESS;
}

void print_lamina(Lamina* lamina) {
    printf("Matriz de temperaturas:\n");
    for (uint64_t i = 0; i < lamina->rows; i++) {
        for (uint64_t j = 0; j < lamina->columns; j++) {
            printf("[%g] ", lamina->temperatures[i][j]);
        }
        printf("\n");
    }
}
void error_manager(Lamina *lamina, shared_file_data *fileobj, const char* error_message) {
    fprintf(stderr, "%s\n", error_message);
    if (lamina) {
        delete_lamina(lamina, fileobj);
    }
}
void delete_lamina(Lamina * lamina, shared_file_data* fileobj) {
    if (lamina) {
        if (lamina->temperatures) {
            for (size_t i = 0; i < lamina->rows; i++) {
                free(lamina->temperatures[i]);
            }
            free(lamina->temperatures);
        }
        if (lamina->next_temperatures) {
            for (size_t i = 0; i < lamina->rows; i++) {
                free(lamina->next_temperatures[i]);
            }
            free(lamina->next_temperatures);
        }
        delete_files(fileobj);
        free(lamina);
    }
}
void format_time(const time_t seconds, char* text, const size_t capacity) {
    struct tm* gmt = gmtime(&seconds);
    size_t lenght = strlen(text);
    snprintf(text + lenght, capacity - lenght, "%04d/%02d/%02d\t%02d:%02d:%02d",
        gmt->tm_year - 70, gmt->tm_mon, gmt->tm_mday - 1, gmt->tm_hour,
            gmt->tm_min, gmt->tm_sec);
}
