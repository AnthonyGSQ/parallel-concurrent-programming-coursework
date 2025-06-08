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
        error_manager(NULL, NULL, "Invalid entrance: you need to specify a"
            ".txt and the amounth of threads you want to use");
        return EXIT_FAILURE;
    }
    // abrimos el archivo txt recibido por parametro
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Error: could not open %s file", argv[1]);
        return EXIT_FAILURE;
    }
    char line[256] = "";
    // while para leer todos los reglones del txt
    while (fgets(line, sizeof(line), file)) {
        Lamina *lamina = (Lamina *)calloc(1, sizeof(Lamina));
        // objeto encargado de manejar los archivos
        shared_file_data * fileobj =
        (shared_file_data*)calloc(1, sizeof(shared_file_data));
        // si no se pudo crear la lamina, retornar EXIT_FAILURE
        if (!lamina) {
            error_manager(lamina, fileobj, "Error: could not allocate memory"
                "for Lamina");
            return EXIT_FAILURE;
        }
        // imprimimos el nombre del archivo txt
        snprintf(fileobj->base_route, sizeof(fileobj->base_route), "%s",
        argv[1]);

        char *lastSLash = strrchr(fileobj->base_route, '/');
        // verificamos la ruta, si es incorrecta, retornar EXIT_FAILURE
        if (!lastSLash) {
            fprintf(stderr, "Error: invalid route %s\n", argv[1]);
            free(lamina);
            return EXIT_FAILURE;
        }
        *(lastSLash + 1) = '\0';
        // si no se pudo leer la linea del txt, retornar EXIT_FAILURE
        if (strcmp(line, "") == 0) {
            error_manager(lamina, fileobj, "Error: could not read a line from"
                "the .txt file");
                return EXIT_FAILURE;
        }
        // Si la cantidad de parametros recibidos no es 5
        // se retorna EXIT_FAILURE
        return_value  = reading_parameters(lamina, fileobj,
            fileobj->binary_file_name,
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
        // llamamos a la funcion encargada de liberar toda la memoria reservada
        delete_lamina(lamina, fileobj);
    }
    // llegados este punto, no hubo fallos y todo fue procesado correctamente
    return EXIT_SUCCESS;
}
int reading_parameters(Lamina *lamina, shared_file_data* fileobj,
    char* bin_file, char* line) {
    char temp[256] = "";
    // si la linea del txt no cuenta con los 5 parametros esperados
    // retorna EXIT_FAILURE
    if (sscanf(line, "%s %" PRIu64 "%lf %lf %lf", temp, &lamina->time,
        &lamina->conductivity,
          &lamina->height, &lamina->epsilon) != 5) {
            error_manager(lamina, fileobj, "Error: The file of configuration"
                "does not have the spected parameters");
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
        error_manager(lamina, fileobj, "Error: Failed to read columns from"
            "file");
        return EXIT_FAILURE;
    }
    // Si el numero de columnas es invalido, se retorna EXIT_FAILURE
    if (lamina->columns <= 0) {
        error_manager(lamina, fileobj, "Error: Invalid number of columns");
        return EXIT_FAILURE;
    }
    // Reservamos memoria para el array que contiene todas las temperaturas
    // tanto las actuales como futuras.
    lamina->temperatures = malloc(sizeof(double) * 2 * lamina->rows *
    lamina->columns);
    if (!lamina->temperatures) {
        error_manager(lamina, fileobj, "Error: No se pudo reservar memoria para"
            "las filas de las matrices.");
            return EXIT_FAILURE;
    }
    // llenamos la matriz con los valores iniciales indicados en el archivo
    // binario
    return_value = fillMatriz(lamina, fileobj);
    // si algo fallo en fillMatriz, retornar EXIT_FAILURE
    if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    // llegados este punto, la lamina fue creada correctamente
    return EXIT_SUCCESS;
}

int fillMatriz(Lamina *lamina, shared_file_data* fileobj) {
    int return_value = 0;
    // bucle para recorrer las filas
    for (size_t i = 0; i < lamina->rows; i++) {
        // bucle para recorrer las columnas
        for (size_t j = 0; j < lamina->columns; j++) {
            // calculamos el indice
            size_t index = i * lamina->columns + j;
            // if para extraer el valor de la posicion actual, si algo falla
            // rompe el ciclo y retornar EXIT_FAILURE
            if (fread(&lamina->temperatures[index], sizeof(double), 1,
            fileobj->file) != 1) {
                error_manager(lamina, fileobj, "Error: Failed to read"
                    "temperature values from file");
                return EXIT_FAILURE;
            }
        }
    }
    // imprimimos la lamina inicial
    //print_lamina(lamina);
    // mandamos a llamar a la funcion update_lamina para comenzar la simulacion
    return_value = update_lamina(lamina, fileobj);
    // si algo fallo en update_lamina, retornar EXIT_FAILURE
    if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    // llegados este punto, la lamina fue creada y estabilizada correctamente
    return EXIT_SUCCESS;
}
int update_lamina(Lamina * lamina, shared_file_data *fileobj) {
    int return_value = 0;
    // variable donde se guarda la diferencia de la celda actual con su futuro
    // estado para compararla con el epsilon y saber si dicha celda esta estable
    double diff = 0;
    int estados = 0;
    // variable para contar cuantas celdas no estan estables, con solo una no
    // estable, el while se vuelve a ejecutar
    size_t unstable_cells = 1;
    // variable para calcular el indice de la matriz actual
    size_t index = 0;
    // variable para calcular el indice de la matriz del siguiente estado
    size_t next_index = 0;
    // numero de celdas totales de la lamina actual
    size_t total_cells = lamina->rows * lamina->columns;
    // variable para acceder a la matriz del estado actual
    size_t current_offset = 0;
    // variable para acceder a la matriz del estado futuro
    size_t next_offset = total_cells;
    // mientras existan celdas inestables en la lamina, se sigue la simulacion
    while (unstable_cells > 0) {
        estados++;
        unstable_cells = 0;
        // bucle para recorrer todas las filas y columnas de la lamina
        for (size_t i = 0; i < lamina->rows; i++) {
            for (size_t j = 0; j < lamina->columns; j++) {
                size_t index = i * lamina->columns + j;
                // current y next hacen swap en cada iteracion para que las temp
                // futuras pasen a ser las actuales en cada iteracion.
                size_t current_index = current_offset + index;
                size_t next_index = next_offset + index;
                // los bordes son constantes, de manera en que, si la celda
                // actual es un borde, se mantiene su temperatura
                if (i == 0 || i == lamina->rows - 1 ||
                    j == 0 || j == lamina->columns - 1) {
                    lamina->temperatures[next_index] =
                    lamina->temperatures[current_index];
                    continue;
                }
                // reseteamos las temperaturas del estado futuro para evitar
                // calculos erroneos
                lamina->temperatures[next_index] = 0;
                // actualizamos la temperatura de la celda actual
                update_cell(lamina, i, j, lamina->temperatures + current_offset,
                    lamina->temperatures + next_offset, lamina->columns);
                // revisamos si la celda actual se encuentra estable, si no esta
                // estabilizada, aumenta el contador de unstable_cells
                // provocando que el while se vuelva a ejecutar
                double diff = fabs(lamina->temperatures[next_index] -
                    lamina->temperatures[current_index]);
                if (diff > lamina->epsilon) {
                    unstable_cells++;
                }
            }
        }
        // swap de las matrices current y next
        size_t temp = current_offset;
        current_offset = next_offset;
        next_offset = temp;
    }
    printf("Estado: %d\n", estados);
    printf("Lamina estabilizada: \n");
    // guardamos la cantidad de estados transcurridos
    lamina->k = estados;
    // actualizamos el tiempo de la lamina, multiplicando el tiempo por la
    // cantidad de estados transcurridos
    lamina->time *= lamina->k;
    // imprimimos la lamina estabilizada
    //print_lamina(lamina);
    // si finish_simulation falla, retornar EXIT_FAILURE
    return_value = finish_simulation(lamina, fileobj);
    if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    // llegados este punto, la lamina fue estabilizada y los archivos fueron
    // reportados correctamente
    return EXIT_SUCCESS;
}
void update_cell(Lamina *lamina, size_t row, size_t column, double *current_mat,
    double *next_mat, size_t cols) {
    size_t idx = row * cols + column;
    // conseguimos las temperaturas de las celdas adyacentes
    double up = current_mat[(row - 1) * cols + column];
    double down = current_mat[(row + 1) * cols + column];
    double left = current_mat[row * cols + (column - 1)];
    double right = current_mat[row * cols + (column + 1)];
    // calculamos la temperatura de la celda en su estado futuro
    next_mat[idx] = current_mat[idx] +
                    ((lamina->time * lamina->conductivity) / (lamina->height *
                        lamina->height)) *
                    (up + down + right + left - 4 * current_mat[idx]);
}
int finish_simulation(Lamina * lamina, shared_file_data *fileobj) {
    // si el archivo tsv con el reporte falla, retornar EXIT_FAILURE
    int return_value = report_file(lamina, fileobj);
    if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    // llegados este punto, el archivo tsv fue creado correctamente
    return EXIT_SUCCESS;
}
int report_file(Lamina *lamina, shared_file_data *fileobj) {
    // variable para guardar el texto que se escribira en el archivo tsv
    // y el nombre del archivo tsv
    // donde se guardara el reporte de la lamina
    char text[260] = "";
    char temp[260] = "";
    // variable con el nombre del archivo tsv donde se escribira el reporte
    char file_name[260] = "";
    char alternative_route[260] = "";
    // creamos y guardamos en file_name el nombre del tsv
    snprintf(file_name, sizeof(file_name), "%s", fileobj->binary_file_name);
    char *lastSLash = strrchr(file_name, '/') + 1;
    char *dot = strrchr(file_name, '.');
    *dot = '\0';
    // creamos una ruta alternativa en caso de que no se encuentre la carpeta
    // tsv, se crea el archivo tsv en la misma carpeta donde se encuentra el
    // binario leido
    snprintf(alternative_route, sizeof(alternative_route), "%s%s%s",
    fileobj->base_route, lastSLash, ".tsv");
    snprintf(temp, sizeof(temp), "%s%s%s%s", fileobj->base_route,
    "tsv/", lastSLash, ".tsv");
    fileobj->report_file = fopen(temp, "w+");
    // verificamos si en efecto no se pudo abrir el archivo tsv
    if (!fileobj->report_file) {
        printf("Error: could not open tsv file\n");
        printf("Creating new tsv file\n");
        fileobj->report_file = fopen(alternative_route, "w+");
    }
    // calculamos el tiempo de la lamina y lo guardamos en el texto
    double time = lamina->time / lamina->k;
    // guardamos en el tsv todos los datos del reporte de la lamina
    snprintf(text, sizeof(temp), "%s.bin\t%g\t%g\t%g\t%g\t%g\t", lastSLash,
    time, lamina->conductivity, lamina->height, lamina->epsilon, lamina->k);
    // formateamos el tiempo a una escritura legible para una persona
    format_time(lamina->time, text, sizeof(text));
    fprintf(fileobj->report_file, "%s", text);
    return EXIT_SUCCESS;
}

void print_lamina(Lamina* lamina) {
    printf("Matriz de temperaturas:\n");
    size_t index = 0;
    // bucle para iterar sobre las filas de la lamina
    for (uint64_t i = 0; i < lamina->rows; i++) {
        // bucle para iterar sobre las columnas de la lamina
        for (uint64_t j = 0; j < lamina->columns; j++) {
            // imprimimos la celda actual
            index = i * lamina->columns + j;
            printf("[%g] ", lamina->temperatures[index]);
        }
        printf("\n");
    }
}
void error_manager(Lamina *lamina, shared_file_data *fileobj,
    const char* error_message) {
    // imprimimos el mensaje de error recibido
    fprintf(stderr, "%s\n", error_message);
    delete_lamina(lamina, fileobj);
}
void delete_lamina(Lamina * lamina, shared_file_data* fileobj) {
    // si existe el objeto lamina, se libera la memoria reservada
    if (lamina) {
        // si existe la matriz de temperaturas, se libera la memoria reservada
        if (lamina->temperatures) {
            free(lamina->temperatures);
        }
        // si existen archivos abiertos, se cierran
        delete_files(fileobj);
        // liberamos la memoria reservada para el objeto lamina
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
