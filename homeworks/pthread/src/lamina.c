// Copyright 2025 Anthony Sanchez
#include "lamina.h"
#include "FileManager.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
/***
 * @file lamina.c
 * @brief Implementacion de la funcion lamina.c
 * @author Anthony Sanchez
 * @date 2025-03-29
 */


/**
 * @brief Constructor de la lamina, se asegura de procesar cada plate del txt
 * 
 * Lee un archivo de texto especificado en los argumentos de la línea de
 * comandos,procesa cada línea y crea una estructura Lamina con los datos
 * obtenidos.
 * 
 * @param argc Número de argumentos de la línea de comandos.
 * @param argv aqui se recibe la ruta del archivo txt a leer
 * 
 * @return EXIT_SUCCESS si la operación fue exitosa, EXIT_FAILURE en caso de 
 *error.
 * 
 * @note Si no se reciben suficientes argumentos o el archivo no se puede
 * abrir, 
 *       la función imprime un mensaje de error y retorna `EXIT_FAILURE`.
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
    char *end;
    size_t thread_count = strtoul(argv[2], &end, 10);
    if (*end != '\0') {
        thread_count = sysconf(_SC_NPROCESSORS_CONF);
        fprintf(stderr, "Warning!: invalid thread_count, taking %zu as"
            "thread_number\n", thread_count);
    }
    if (!file) {
        fprintf(stderr, "Error: could not open %s file", argv[1]);
        return EXIT_FAILURE;
    }
    char line[256] = "";
    // while para leer todos los reglones del txt
    while (fgets(line, sizeof(line), file)) {
        Lamina *lamina = (Lamina *)calloc(1, sizeof(Lamina));
        lamina->thread_count = thread_count;
        file_struct * fileobj = (file_struct*)calloc(1,sizeof(file_struct));
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
        return_value  = reading_parameters(lamina, fileobj,
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

/**
 * @brief Lee y valida los parámetros de la lamina
 * 
 * Extrae los valores tiempo, conductividad, height y epsilon, guardandolos en
 * el struct lamina
 * 
 * @param lamina Puntero a la estructura Lamina que tiene todos los parametros
 * @param bin_file Nombre del archivo binario generado.
 * @param line Línea de texto con los parámetros a leer.
 * 
 * @return EXIT_SUCCESS si encuentra los parametros, EXIT_FAILURE si no
 * 
 * @note Si la línea no tiene los parámetros esperados o alguno de los valores
 * es inválido, 
 *       la función imprime un mensaje de error y retorna `EXIT_FAILURE`.
 */
int reading_parameters(Lamina *lamina, file_struct* fileobj, char* line) {
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
/**
 * @brief Inicializa la matriz del plate actual.
 * 
 * Lee los valores de filas y columnas desde un archivo binario,
 * asigna memoria para las matrices de temperaturas y carga sus valores.
 * Luego, llama a `update_lamina()`.
 * 
 * @param lamina Puntero a la estructura Lamina
 * 
 * @return EXIT_SUCCESS si pudo crear la matriz, EXIT_FAILURE si no
 * 
 * @note Si la lectura del archivo falla o la reserva de memoria no es exitosa, 
 *       la función imprime un mensaje de error y retorna `EXIT_FAILURE`.
 */
int create_lamina(Lamina *lamina, file_struct* fileobj) {
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
        lamina->temperatures[i] = (double *)malloc(lamina->columns *
            sizeof(double));
        lamina->next_temperatures[i] = (double *)malloc(lamina->columns *
            sizeof(double));

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
    plan_thread_distribution(lamina, fileobj);
    return EXIT_SUCCESS;
}

int fillMatriz(Lamina *lamina, file_struct* fileobj) {
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

void plan_thread_distribution(Lamina *lamina, file_struct * fileobj) {
    public_data_t *public_data = (public_data_t*)calloc(1,
        sizeof(public_data_t));
    double total_cells = lamina->rows * lamina->columns;
    // en caso de haber menos de 100 celdas, no vale la pena crear n hilos
    // para realizar el trabajo de la matriz debido al overhead
    if (total_cells < 100) {
        printf("Warning!: Not enought cells to make the use of threads"
            " worth it, instead the program is gonna use only one thread\n");
        lamina->thread_count = 1;
        return;
    }
    // Llegados aqui, existen suficientes celdas para considerar la creacion de
    // los hilos solicitados para el procesamiento de la matriz

    if (lamina->thread_count > total_cells) {
        lamina->thread_count = total_cells;
    }
    size_t cells_per_thread = total_cells / lamina->thread_count;
    if ( cells_per_thread < 100) {
        cells_per_thread = 100;
        lamina->thread_count = total_cells / 100;
    }
    printf("Total rows: % " PRIu64 "\n", lamina->rows);
    printf("Total columns %" PRIu64 ":\n", lamina->columns);
    printf("Número total de celdas: %f\n", total_cells);
    printf("Número de hilos: %f\n", lamina->thread_count);
    printf("Celdas por hilo (mínimo 100): %zu\n", cells_per_thread);
    public_data->x_increment = lamina->rows/lamina->thread_count;
    public_data->y_increment = lamina->columns/lamina->thread_count;
    printf("Filas por hilo: %zu\n", public_data->x_increment);
    printf("Columnas por hilo: %zu\n", public_data->y_increment);
}

/**
 * @brief Actualiza la matriz de temperaturas hasta estabilizarla
 * 
 * Recorre la matriz de temperaturas, actualizando cada celda hasta que todas
 * sean estables (que tengan una diferencia entre el futuro estado y el actual
 * menor a epislon). Intercambia las matrices `temperatures` y
 * next_temperatures` en cada iteración.
 * 
 * @param lamina Puntero a la estructura lamina
 * 
 * @return EXIT_SUCCESS si estabilizo la lamina, EXIT_FAILURE si no
 * 
 * @note Si la simulación falla al finalizar, la función devuelve
 * `EXIT_FAILURE`.
 */
int update_lamina(Lamina * lamina, file_struct *fileobj) {
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
        // TODO(AnthonyGSQ): quiza no es necesario este doble for
        for (size_t i = 0; i < lamina->rows; i++) {
            for (size_t j = 0; j < lamina->columns; j++) {
                if (i == 0 || i == lamina->rows - 1 ||
                        j == 0 || j == lamina->columns -1) {
                    continue;

                }
                lamina->next_temperatures[i][j] = 0;
            }
        }

        for (size_t i = 0; i < lamina->thread_count; i++) {
                // aqui se crean los threads

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
// la idea sera que esta funcion sea la de for for de cada hilo
// los bordes que los lea desde el private data de cada hilo

void* update_lamina_block(void* data) {
    Lamina lamina = *(Lamina*) data;
    size_t start_row = lamina.public_data->private_data->x1;
    size_t finish_row = lamina.public_data->private_data->x2;
    size_t start_column = lamina.public_data->private_data->y1;
    size_t finish_column = lamina.public_data->private_data->y2;
    for (size_t i = start_row; i < finish_row; i++){
        for(size_t j = start_column; j < finish_column; j++){
            // if para ignorar los bordes de la matriz
            if (i == 0 || i == lamina.rows - 1 ||
                j == 0 || j == lamina.columns -1) {
                  continue;
            }
            update_cell(&lamina, i ,j);
        }
    }
    return;
}
/**
 * @brief Actualiza la temperatura de la celda recibida.
 * 
 * Calcula la nueva temperatura de la celda en la posición (`row`, `column`)
 * basada en el método de diferencias finitas, utilizando las temperaturas de
 * sus celdas adyacentes.
 * 
 * @param lamina Puntero a la estructura lamina
 * @param row fila de la celda a actualizar
 * @param column columna de la celda a actualizar
 * 
 * @note Solo se actualizan las celdas internas de la matriz, evitando los
 * bordes.
 */
int update_cell(Lamina * lamina, uint64_t row, uint64_t column) {
    // aseguramos que no se procesara una celda invalida
    size_t unstable_cells = 0;
    double diff = 0;
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
        diff = fabs(lamina->next_temperatures[row][column] -
            lamina->temperatures[row][column]);
        // SI la celda actual no esta estable, unstable_cells aumenta
        // su valor, asegurando que la lamina sea procesada un estado
        // mas
        if (diff > lamina->epsilon) {
            unstable_cells++;
        }
    }
    return unstable_cells;
}
/**
 * @brief Finaliza la simulación y llama a report_file
 * 
 * Llama a `report_file` para escribir el resultado de la simulación. 
 * Si la generación del reporte falla, retorna `EXIT_FAILURE`.
 * 
 * @param lamina Puntero a la estructura lamina
 * @param fileobj PUntero a la estructura de archivos
 * @return `EXIT_SUCCESS` si genero el reporte, EXIT_FAILURE si no
 */
int finish_simulation(Lamina * lamina, file_struct *fileobj) {
    int return_value = report_file(lamina, fileobj);
    if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
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
/**
 * @brief Maneja casi todos los errores, llamando a delete_lamina luego
 *
 * Esta funcion recibe el mensaje a imprimir como reporte del error, una vez
 * hecho eso, llama la funcion delete_lamina() para liberar toda la memoria
 * aun no liberada
 *
 * @param lamina Un puntero a la estructura lamina
 * @param error_message Un mensaje de error que se imprime en la salida de
 * error estándar.
 */
void error_manager(Lamina *lamina, file_struct *fileobj, const char* error_message) {
    fprintf(stderr, "%s\n", error_message);
    if (lamina) {
        delete_lamina(lamina, fileobj);
    }
}
/**
 * @brief Libera los recursos de lamina
 *
 * Esta función libera la memoria dinámica utilizada por la estructura `Lamina`
 * Antes de hacerle free verifica que exista lo que se piensa borrar para evitar
 * intentar liberar memoria ya liberada o que nunca fue reservada.
 *
 * @param lamina Un puntero a la estructura lamina, si no existe, no hace nada.
 */
void delete_lamina(Lamina * lamina, file_struct* fileobj) {
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
/**
 * @brief Formatea el tiempo en segundos a una cadena legible para humanos
 *
 * Esta función convierte el tiempo en segundos recibidos en un formato legible
 * para un ser humano: "YYYY/MM/DD hh:mm:SS".
 *
 * @param seconds Cantidad de segundos desde la época Unix a convertir a
 * formato de tiempo.
 * @param text Puntero a una cadena de caracteres donde se almacenará el tiempo
 * formateado.
 * @param capacity Tamaño máximo de la cadena `text`.
 */
void format_time(const time_t seconds, char* text, const size_t capacity) {
    struct tm* gmt = gmtime(&seconds);
    size_t lenght = strlen(text);
    snprintf(text + lenght, capacity - lenght, "%04d/%02d/%02d\t%02d:%02d:%02d",
        gmt->tm_year - 70, gmt->tm_mon, gmt->tm_mday - 1, gmt->tm_hour,
            gmt->tm_min, gmt->tm_sec);
}
