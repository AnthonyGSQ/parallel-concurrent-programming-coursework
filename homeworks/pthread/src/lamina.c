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
#include <assert.h>
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
        error_manager(NULL, NULL,
            "Invalid entrance: you need to specify a .txt and"
            "the amounth of threads you want to use");
        return EXIT_FAILURE;
    }
    int64_t temp = 0;
    FILE *file = fopen(argv[1], "r");
    if (argc == 3) {
        char *end;
        temp = strtoul(argv[2], &end, 10);
        // si el usuario envia nada, negativo o cero, cae en este if
        if (*end != '\0' || temp <= 0) {
            temp = sysconf(_SC_NPROCESSORS_CONF);
            fprintf(stderr,
                "Warning!: invalid thread_count, taking %" PRId64 " as "
                "thread_number\n", temp);
        }
    } else {
        temp = sysconf(_SC_NPROCESSORS_CONF);
    }
    if (!file) {
        fprintf(stderr, "Error: could not open %s file", argv[1]);
        return EXIT_FAILURE;
    }
    char line[256] = "";
    size_t thread_count = (size_t)temp;
    // while para leer todos los reglones del txt
    printf("THREAD_COUNT: %" PRId64 "\n", thread_count);
    while (fgets(line, sizeof(line), file)) {
        Lamina *lamina = (Lamina *)calloc(1, sizeof(Lamina));
        public_data_t *public_data = calloc(1, sizeof(public_data_t));
        public_data->thread_count = thread_count;
        file_struct * fileobj = (file_struct*)calloc(1, sizeof(file_struct));
        if (!lamina) {
            error_manager(lamina, fileobj,
                "Error: could not allocate memory for"
                "Lamina");
            return EXIT_FAILURE;
        }
        snprintf(fileobj->base_route, sizeof(fileobj->base_route),
        "%s", argv[1]);

        char *lastSLash = strrchr(fileobj->base_route, '/');
        if (!lastSLash) {
            fprintf(stderr, "Error: invalid route %s\n", argv[1]);
            free(lamina);
            return EXIT_FAILURE;
        }
        *(lastSLash + 1) = '\0';
        if (strcmp(line, "") == 0) {
            error_manager(lamina, fileobj,
                "Error: could not read a line from the .txt"
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
        return_value = create_lamina(lamina, fileobj, public_data);
        if (return_value == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        return_value = update_lamina(lamina, fileobj, public_data);
        if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
        free(public_data);
        delete_lamina(lamina, fileobj);
    }
    return EXIT_SUCCESS;
}
int reading_parameters(Lamina *lamina, file_struct* fileobj, char* line) {
    char temp[256] = "";
    // si la linea del txt no cuenta con los 5 parametros esperados
    // retorna EXIT_FAILURE
    if (sscanf(line, "%s %lf %lf %lf %lf", temp, &lamina->time,
        &lamina->conductivity,
          &lamina->height, &lamina->epsilon) != 5) {
            error_manager(lamina, fileobj,
                "Error: The file of configuration does not"
                "have the spected parameters");
                return EXIT_FAILURE;
    }
    // se agrega la ruta al binario para poder leer el archivo
  snprintf(fileobj->binary_file_name,
      sizeof(fileobj->binary_file_name) + sizeof(temp), "%s%s",
      fileobj->base_route, temp);
    // Ifs que verifican que los valores recibidos del txt sean validos, si no
    // lo son, retornan EXIT_FAILURE
    printf("Time: %.f\n", lamina->time);
    if (lamina->time == 0.0) {
        error_manager(lamina, fileobj, "Invalid time");
        return EXIT_FAILURE;
    }
    printf("Conductivity: %.3f\n", lamina->conductivity);
    if (lamina->conductivity <= 0.0) {
        error_manager(lamina, fileobj, "Invalid conductivity");
        return EXIT_FAILURE;
    }
    printf("Height: %.f\n", lamina->height);
    if (lamina->height <= 0.0) {
        error_manager(lamina, fileobj, "Invalid height");
        return EXIT_FAILURE;
    }
    printf("Epsilon: %.15f\n", lamina->epsilon);
    if (lamina->epsilon <= 0.0) {
        error_manager(lamina, fileobj, "Invalid epsilon");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
int create_lamina(Lamina *lamina, file_struct* fileobj,
    public_data_t* public_data) {
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
        error_manager(lamina, fileobj,
            "Error: Failed to read columns from file");
        return EXIT_FAILURE;
    }
    // Si el numero de columnas es invalido, se retorna EXIT_FAILURE
    if (lamina->columns <= 0) {
        error_manager(lamina, fileobj, "Error: Invalid number of columns");
        return EXIT_FAILURE;
    }
    // Reservamos memoria para las filas de las dos matrices de la lamina
    lamina->temperatures = (double **)calloc(lamina->rows, sizeof(double *));
    lamina->next_temperatures = (double **)calloc(lamina->rows,
        sizeof(double*));
    if (!lamina->temperatures || !lamina->next_temperatures) {
        error_manager(lamina, fileobj,
            "Error: No se pudo reservar memoria para las"
            "filas de las matrices.");
            return EXIT_FAILURE;
    }

    // Reservar memoria para cada fila
    for (size_t i = 0; i < lamina->rows; i++) {
        lamina->temperatures[i] = (double *)calloc(lamina->columns,
            sizeof(double));
        lamina->next_temperatures[i] = (double *)calloc(lamina->columns,
            sizeof(double));

        // Comprobar si la reserva de memoria falló para alguna fila
        if (!lamina->temperatures[i] || !lamina->next_temperatures[i]) {
            error_manager(lamina, fileobj,
                "Error: No se pudo reservar memoria para las"
                "filas de las matrices.");
                return EXIT_FAILURE;
        }
    }
    return_value = fillMatriz(lamina, fileobj);
    if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    plan_thread_distribution(lamina, public_data);
    return EXIT_SUCCESS;
}
int fillMatriz(Lamina *lamina, file_struct* fileobj) {
    // Bucle para leer los valores de la matriz
    for (size_t i = 0; i < lamina->rows; i++) {
        for (size_t j = 0; j < lamina->columns; j++) {
            if (fread(&lamina->temperatures[i][j], sizeof(double), 1,
            fileobj->file) != 1) {
                error_manager(lamina, fileobj,
                    "Error: Failed to read temperature values"
                    "from file");
                return EXIT_FAILURE;
            }
        }
    }
    // TODO(AnthonyGSQ): Impresion temporal, es solo para ver que todo sirva
    // hasta aca
    print_lamina(lamina);
    return EXIT_SUCCESS;
}

void plan_thread_distribution(Lamina *lamina,
    public_data_t* public_data) {
    double total_cells = lamina->rows * lamina->columns;
    // en caso de haber menos de 100 celdas, no vale la pena crear n hilos
    // para realizar el trabajo de la matriz debido al overhead
    if (total_cells < 100) {
        printf("Warning!: Not enought cells to make the use of threads"
            " worth it, instead the program is gonna use only one thread\n");
        public_data->thread_count = 1;
        public_data->x_increment = lamina->rows;
        public_data->y_increment = lamina->columns;
        return;
    }
    // Llegados aqui, existen suficientes celdas para considerar la creacion de
    // los hilos solicitados para el procesamiento de la matriz

    if (public_data->thread_count > total_cells) {
        public_data->thread_count = total_cells;
    }
    size_t cells_per_thread = total_cells / public_data->thread_count;
    if (cells_per_thread < 100) {
        cells_per_thread = 100;
        public_data->thread_count = total_cells / 100;
    }

    public_data->x_increment = lamina->rows/public_data->thread_count;
    public_data->y_increment = lamina->columns/public_data->thread_count;
}
int update_lamina(Lamina * lamina, file_struct *fileobj,
    public_data_t* public_data) {
    int return_value = 0;
    public_data->lamina = lamina;

    size_t unstable_blocks = 1;
    int estados = 0;
    double** temp = NULL;
    size_t thread_count = public_data->thread_count;
    // creamos n private_datas para los n threads
    private_data_t* private_data = calloc(thread_count,
        sizeof(private_data_t));
    if (!private_data) {
        error_manager(lamina, fileobj,
            "Error: could not make calloc for private_data");
        return EXIT_FAILURE;
    }
    // creamos los n hilos
    pthread_t* threads = calloc(thread_count, sizeof(pthread_t));
    if (!threads) {
        error_manager(lamina, fileobj,
            "Error: could not make calloc for threads");
        free(private_data);
        return EXIT_FAILURE;
    }
    // bucle para asignarle el thread_num y el public data a cada private data
    for (size_t i = 0; i < thread_count; i++) {
        private_data[i].thread_num = i;
        private_data[i].public_data = public_data;
    }
    while (unstable_blocks > 0) {
        estados++;
        unstable_blocks = 0;
        // bucle para asignarle trabajo a cada hilo
        for (size_t i = 0; i < thread_count; i++) {
            // si algo falla liberamos toda la memoria reservada
            if (pthread_create(&threads[i], NULL, update_lamina_block,
                &private_data[i]) != 0) {
                error_manager(lamina, fileobj, "Error: pthread_create falló");
                free(threads);
                free(private_data);
                return EXIT_FAILURE;
            }
        }
        // ciclo para el threads_join y recopilacion de cuantos bloques
        // no estables existen aun
        for (size_t i = 0; i < public_data->thread_count; i++) {
            pthread_join(threads[i], NULL);
            unstable_blocks += private_data[i].unstable_cells;
        }
        // swap de las matrices
        temp = lamina->temperatures;
        lamina->temperatures = lamina->next_temperatures;
        lamina->next_temperatures = temp;
        printf("Estado: %d\n", estados);
        //lamina->k += lamina->time;
    }
    // una vez terminado el trabajo de los threads, liberamos la memoria
    // reservada
    free(threads);
    free(private_data);
    printf("Estado: %d\n", estados);
    lamina->k = estados;
    lamina->time *= lamina->k;
    printf("Lamina estabilizada: \n");
    print_lamina(lamina);

    return_value = finish_simulation(lamina, fileobj);
    if (return_value == EXIT_FAILURE) {
        printf("[ERROR] finish_simulation falló\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
void* update_lamina_block(void* data) {
    assert(data);
    private_data_t *private_data = (private_data_t*) data;
    size_t x_increment = private_data->public_data->x_increment;
    size_t y_increment = private_data->public_data->y_increment;
    // Dividir solo por filas (x), no por columnas
    private_data->x1 = private_data->thread_num * x_increment;
    private_data->x2 = (private_data->thread_num ==
        private_data->public_data->thread_count - 1)
        ? private_data->public_data->lamina->rows
        : (private_data->thread_num + 1) * x_increment;
    
    private_data->y1 = 0;
    private_data->y2 = private_data->public_data->lamina->columns;
    

    
    /*
    private_data->y1 = private_data->thread_num * y_increment;
    private_data->y2 = (private_data->thread_num ==
        private_data->public_data->thread_count - 1)
        ? private_data->public_data->lamina->columns
        : (private_data->thread_num + 1) * y_increment;

    */
    //printf("THREAD: %zu\t X1: %zu\t X2: %zu\t Y1: %zu\t Y2: %zu\n",
    //    private_data->thread_num, private_data->x1, private_data->x2,
    //        private_data->y1, private_data->y2);
    size_t unstable_cells = 0;
    // recorrido del bloque correspondiente de cada hilo, que ignora los bordes
    // pues estos son constantes
    for (size_t i = private_data->x1; i < private_data->x2; i++) {
        for (size_t j = private_data->y1; j < private_data->y2; j++) {
            if (i == 0 || i == private_data->public_data->lamina->rows - 1 ||
                j == 0 || j == private_data->public_data->lamina->columns - 1) {
                private_data->public_data->lamina->next_temperatures[i][j] =
                private_data->public_data->lamina->temperatures[i][j];
                continue;
            }
            private_data->public_data->lamina->next_temperatures[i][j] = 0;
            update_cell(private_data->public_data->lamina, i, j,
                &unstable_cells);
        }
    }
    private_data->unstable_cells = unstable_cells;
    return NULL;
}
void update_cell(Lamina * lamina, size_t row, size_t column,
    size_t *unstable_cells) {
    double diff = 0;
    // aseguramos que no vamos a procesar un borde
    if (row > 0 && row < lamina->rows - 1 && column > 0 &&
        column < lamina->columns - 1) {
        // obtenemos los valores de las celdas adyacentes a la actual
        double up = lamina->temperatures[row - 1][column];
        double down = lamina->temperatures[row + 1][column];
        double left = lamina->temperatures[row][column - 1];
        double right = lamina->temperatures[row][column + 1];
        double actual = lamina->temperatures[row][column];
        // calculamos la temperatura futura de la celda
        double new_temp = actual +
            ((lamina->time * lamina->conductivity) /
            (lamina->height * lamina->height)) *
            (up + down + right + left - (4 * actual));
        lamina->next_temperatures[row][column] = new_temp;
        diff = fabs(new_temp - actual);
        if (diff > lamina->epsilon) {
            (*unstable_cells)++;
        }
    }
}
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
void error_manager(Lamina *lamina, file_struct *fileobj,
    const char*error_message) {
    fprintf(stderr, "%s\n", error_message);
    if (lamina) {
        delete_lamina(lamina, fileobj);
    }
}
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
void format_time(const time_t seconds, char* text, const size_t capacity) {
    struct tm* gmt = gmtime(&seconds);
    size_t lenght = strlen(text);
    snprintf(text + lenght, capacity - lenght, "%04d/%02d/%02d\t%02d:%02d:%02d",
        gmt->tm_year - 70, gmt->tm_mon, gmt->tm_mday - 1, gmt->tm_hour,
            gmt->tm_min, gmt->tm_sec);
}
