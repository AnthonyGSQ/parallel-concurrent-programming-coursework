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
    printf("THREAD_COUNT: %" PRId64 "\n", thread_count);
    // while para leer todos los reglones del txt
    while (fgets(line, sizeof(line), file)) {
        // creamos el objeto lamina
        Lamina *lamina = (Lamina *)calloc(1, sizeof(Lamina));
        // creamos el struct de datos compartidos entre hilos
        public_data_t *public_data = calloc(1, sizeof(public_data_t));
        public_data->thread_count = thread_count;
        file_struct * fileobj = (file_struct*)calloc(1, sizeof(file_struct));
        // si la lamina no pudo ser creada, retornar EXIT_FAILURE
        if (!lamina) {
            error_manager(lamina, fileobj,
                "Error: could not allocate memory for"
                "Lamina");
            return EXIT_FAILURE;
        }
        snprintf(fileobj->base_route, sizeof(fileobj->base_route),
        "%s", argv[1]);

        char *lastSLash = strrchr(fileobj->base_route, '/');
        // si la ruta es invalida, retornar EXIT_FAILURE
        if (!lastSLash) {
            fprintf(stderr, "Error: invalid route %s\n", argv[1]);
            free(lamina);
            return EXIT_FAILURE;
        }
        *(lastSLash + 1) = '\0';
        // si no se pudo leer la linea del txt, retornar EXIT_FAILURE
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
        // abrimos el archivo binario indicado en el txt
        fileobj->file = fopen(fileobj->binary_file_name, "rb");
        printf("Creating lamina...\n");
        // si algo fallo en la creacion de la lamina o en la estabilizacion
        // de la misma, se retorna EXIT_FAILURE
        return_value = create_lamina(lamina, fileobj, public_data);
        if (return_value == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        // si no se pudo iniciar los hilos, o algo fallo en la simulacion
        // retornar EXIT_FAILURE
        return_value = starThreads(lamina, fileobj, public_data);
        if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
        // liberamos la memoria de los datos compartidos
        free(public_data);
        delete_lamina(lamina, fileobj);
    }
    // llegados este punto, todo salio bien
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
    // llamamos a llenar la matriz con los valores del binario, si algo falla
    // retornar EXIT_FAILURE
    return_value = fillMatriz(lamina, fileobj);
    if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    // calculamos la cantidad de hilos a crear y de donde a donde debe de
    // recorrer cada hilo
    plan_thread_distribution(lamina, public_data);
    return EXIT_SUCCESS;
}
int fillMatriz(Lamina *lamina, file_struct* fileobj) {
    // Bucle para leer los valores de la matriz
    for (size_t i = 0; i < lamina->rows; i++) {
        for (size_t j = 0; j < lamina->columns; j++) {
            // leemos el valor de la temperatura de la celda, si algo fallo
            // salimos del bucle y retornamos EXIT_FAILURE
            if (fread(&lamina->temperatures[i][j], sizeof(double), 1,
            fileobj->file) != 1) {
                error_manager(lamina, fileobj,
                    "Error: Failed to read temperature values"
                    "from file");
                return EXIT_FAILURE;
            }
        }
    }
    print_lamina(lamina);
    return EXIT_SUCCESS;
}
void plan_thread_distribution(Lamina *lamina,
    public_data_t* public_data) {
    // cantidad total de celdas que tiene la lamina actual
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
    // calculamos la cantidad de celdas que debe procesar cada hilo
    size_t cells_per_thread = total_cells / public_data->thread_count;
    // si la cantidad de celdas por hilo es menor a 100, se ajusta
    // para que cada hilo procese al menos 100 celdas, y se ajusta la cantidad
    // de hilos a crear
    if (cells_per_thread < 100) {
        cells_per_thread = 100;
        public_data->thread_count = total_cells / 100;
    }
    // calculamos el incremento de filas y columnas que debe procesar cada hilo
    public_data->x_increment = lamina->rows/public_data->thread_count;
    public_data->y_increment = lamina->columns/public_data->thread_count;
}
int starThreads(Lamina *lamina, file_struct *fileobj,
    public_data_t* public_data) {
    // asignamos el objeto lamina a los datos compartidos entre hilos
    public_data->lamina = lamina;
    // cantidad de hilos a crear
    size_t thread_count = public_data->thread_count;
    // creamos thread_count structs para cada hilo
    private_data_t* private_data = calloc(thread_count, sizeof(private_data_t));
    // si no se pudieron crear, retorna EXIT_FAILURE
    if (!private_data) {
        error_manager(lamina, fileobj,
            "Error: could not make calloc for private_data");
        return EXIT_FAILURE;
    }
    // creamos los thread_count hilos
    pthread_t* threads = calloc(thread_count, sizeof(pthread_t));
    // si no se pudieron crear, retorna EXIT_FAILURE;
    if (!threads) {
        error_manager(lamina, fileobj,
            "Error: could not make calloc for threads");
        return EXIT_FAILURE;
    }
    // llamar a update lamina
    // bucle para asignarle trabajo a cada hilo
    pthread_barrier_init(&public_data->barrier, NULL,
        public_data->thread_count);
    // inicializamos para la primera iteracion del while de update_lamina
    public_data->unstable_blocks = 1;
    // inicializamos en 0 los estados de la lamina, para evitar calculos
    // erroneos
    public_data->estados = 0;
    // inicializamos el puntero temporal para el swap en NULL
    public_data->temp = NULL;
        // bucle para la creacion de hilos
        for (size_t i = 0; i < thread_count; i++) {
            // si algo falla liberamos toda la memoria reservada
            private_data[i].thread_num = i;
            private_data[i].public_data = public_data;
            if (pthread_create(&threads[i], NULL, update_lamina,
                &private_data[i]) != 0) {
                error_manager(lamina, fileobj, "Error: pthread_create falló");
                free(threads);
                free(private_data);
                return EXIT_FAILURE;
            }
        }
    // for para destruir todos los hilos creados
    for (size_t i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    // liberamos toda la memoria relacionada a los threads
    free(threads);
    // liberamos la memoria reservada para los datos privados
    free(private_data);
    printf("\n\n");
    printf("Estado: %zu\n\n", public_data->estados);
    // guardamos la cantidad de estados transcurridos en la lamina
    lamina->k = public_data->estados;
    // calculamos el tiempo total de la simulacion
    lamina->time *= lamina->k;
    printf("Lamina estabilizada: \n");
    int return_value = 0;
    // si al finalizar la simulacion algo falla, retornar EXIT_FAILURE
    return_value = finish_simulation(lamina, fileobj);
    if (return_value == EXIT_FAILURE) {
        printf("[ERROR] finish_simulation falló\n");
        return EXIT_FAILURE;
    }
    // imprimimos la lamina final
    print_lamina(lamina);
    return EXIT_SUCCESS;
}

void* update_lamina(void *data) {
    assert(data);
    private_data_t *private_data = (private_data_t*) data;
    while (1) {
        // actualizar el bloque correspondiente a cada hilo
        update_lamina_block(private_data);
        // hacemos que solo un hilo pueda incrementar los estados y
        // resetear los bloques inestables, agregando una barrera para evitar
        // indeterminismo con la cantidad de bloques inestables
        if (pthread_barrier_wait(&private_data->public_data->barrier) ==
            PTHREAD_BARRIER_SERIAL_THREAD) {
                // incrementamos estados una vez por iteracion
            private_data->public_data->estados++;
            // reseteamos los bloques inestables para la siguiente iteracion
            private_data->public_data->unstable_blocks = 0;
        }
        // esperamos a los hilos para que terminen de actualizar sus bloques
        pthread_barrier_wait(&private_data->public_data->barrier);
        // mutex para que solo un hilo pueda modificar unstable_blocks a la vez
        // evitando condiciones de carreras
        pthread_mutex_lock(&private_data->public_data->can_add_unstable_blocks);
            private_data->public_data->unstable_blocks +=
                private_data->unstable_cells;
        // liberamos el mutex
        pthread_mutex_unlock(&private_data->public_data->
            can_add_unstable_blocks);
        // esperamos a que todos los hilos terminen de actualizar sus bloques
        // y luego hacemos el swap de las matrices
        // si es el hilo serial, hacemos el swap de las matrices
        if (pthread_barrier_wait(&private_data->public_data->barrier) ==
            PTHREAD_BARRIER_SERIAL_THREAD) {
            // swap de las matrices
            private_data->public_data->temp =
                private_data->public_data->lamina->temperatures;
            private_data->public_data->lamina->temperatures =
                private_data->public_data->lamina->next_temperatures;
            private_data->public_data->lamina->next_temperatures =
                private_data->public_data->temp;
            printf("Estado: %zu\n", private_data->public_data->estados);
        }
        // esperamos a que todos los hilos tengan el swap de las matrices
        pthread_barrier_wait(&private_data->public_data->barrier);
        // condicion de parada
        pthread_mutex_lock(&private_data->public_data->can_add_unstable_blocks);
        int should_stop = (private_data->public_data->unstable_blocks == 0);
        pthread_mutex_unlock(&private_data->public_data->
            can_add_unstable_blocks);
        // si no hay bloques inestables, se detiene la simulacion
        if (should_stop) {
            break;
        }
    }
    return NULL;
}
void update_lamina_block(private_data_t* private_data) {
    size_t x_increment = private_data->public_data->x_increment;
    size_t y_increment = private_data->public_data->y_increment;
    // Dividir solo por filas (x), no por columnas
    private_data->x1 = private_data->thread_num * x_increment;
    private_data->x2 = (private_data->thread_num ==
        private_data->public_data->thread_count - 1)
        ? private_data->public_data->lamina->rows
        : (private_data->thread_num + 1) * x_increment;
    // cada hilo procesara todas las columnas de su bloque
    private_data->y1 = 0;
    private_data->y2 = private_data->public_data->lamina->columns;
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
    return;
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
        // calculamos la diferencia de temperaturas de la celda actual
        diff = fabs(new_temp - actual);
        // si la diferencia es mayor al epsilon, aumenta la cantidad de
        // unstable cells
        if (diff > lamina->epsilon) {
            (*unstable_cells)++;
        }
    }
}
int finish_simulation(Lamina * lamina, file_struct *fileobj) {
    // si el archivo de reporte falla, retorna EXIT_FAILURE
    int return_value = report_file(lamina, fileobj);
    if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
void print_lamina(Lamina* lamina) {
    return;
    printf("Matriz de temperaturas:\n");
    // bucle para las filas de la matriz
    for (uint64_t i = 0; i < lamina->rows; i++) {
        // bucle para las columnas de la matriz
        for (uint64_t j = 0; j < lamina->columns; j++) {
            printf("[%g] ", lamina->temperatures[i][j]);
        }
        printf("\n");
    }
}
void error_manager(Lamina *lamina, file_struct *fileobj,
    const char*error_message) {
    // imprimimos el mensaje de error recibido
    fprintf(stderr, "%s\n", error_message);
    delete_lamina(lamina, fileobj);
}
void delete_lamina(Lamina * lamina, file_struct* fileobj) {
    // si el objeto lamina existe, se libera la memoria de sus atributos
    if (lamina) {
        // si la lamina existe, la libera
        if (lamina->temperatures) {
            for (size_t i = 0; i < lamina->rows; i++) {
                free(lamina->temperatures[i]);
            }
            free(lamina->temperatures);
        }
        // si la matriz futura existe, la libera
        if (lamina->next_temperatures) {
            for (size_t i = 0; i < lamina->rows; i++) {
                free(lamina->next_temperatures[i]);
            }
            free(lamina->next_temperatures);
        }
        // llamado al destructor de los archivos
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
