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
 * @brief Implementacion de la clase lamina.c
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
    // variable temporal para guardar la cantidad de hilos a crear
    int64_t temp = 0;
    FILE *file = fopen(argv[1], "r");
    // si existe un tercer argumento, intentar tomarlo como cantidad de hilos
    // a crear, si falla, se asume la cantidad de hilos igual a la cantidad
    // de hilos que tenga el dispositivo en donde se compile el programa
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
    // si no se pudo abrir el archivo txt, retornar EXIT_FAILURE
    if (!file) {
        fprintf(stderr, "Error: could not open %s file", argv[1]);
        return EXIT_FAILURE;
    }
    // variable para guardar la linea leida del txt
    char line[256] = "";
    size_t thread_count = (size_t)temp;
    // while para leer todos los reglones del txt
    printf("THREAD_COUNT: %" PRId64 "\n", thread_count);
    while (fgets(line, sizeof(line), file)) {
        // creamos un objeto lamina
        Lamina *lamina = (Lamina *)calloc(1, sizeof(Lamina));
        // creamos el struct de datos compartidos entre hilos
        public_data_t *public_data = calloc(1, sizeof(public_data_t));
        public_data->thread_count = thread_count;
        file_struct * fileobj = (file_struct*)calloc(1, sizeof(file_struct));
        // si no se pudo crear la lamina, retorna EXIT_FAILURE
        if (!lamina) {
            error_manager(lamina, fileobj,
                "Error: could not allocate memory for"
                "Lamina");
            return EXIT_FAILURE;
        }
        snprintf(fileobj->base_route, sizeof(fileobj->base_route),
        "%s", argv[1]);

        char *lastSLash = strrchr(fileobj->base_route, '/');
        // si la ruta no es valida, retorna EXIT_FAILURE
        if (!lastSLash) {
            fprintf(stderr, "Error: invalid route %s\n", argv[1]);
            free(lamina);
            return EXIT_FAILURE;
        }
        *(lastSLash + 1) = '\0';
        // si no se pudo leer la linea del txt, retorna EXIT_FAILURE
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
        // abrimos el binario indicado en la linea del txt ya leida
        fileobj->file = fopen(fileobj->binary_file_name, "rb");
        printf("Creating lamina...\n");
        // si algo fallo en la creacion de la lamina o en la estabilizacion
        // de la misma, se retorna EXIT_FAILURE
        return_value = create_lamina(lamina, fileobj, public_data);
        if (return_value == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        // si algo fallo en la creacion de los hilos o en la simulacion
        // retorna EXIT_FAILURE
        return_value = starThreads(lamina, fileobj, public_data);
        if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
        // liberamos el struct de datos compartidos
        free(public_data);
        // llamamos a liberar el resto de recursos
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
    return_value = fillMatriz(lamina, fileobj);
    if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    plan_thread_distribution(lamina, public_data);
    return EXIT_SUCCESS;
}

int fillMatriz(Lamina *lamina, file_struct* fileobj) {
    // bucle para recorrer las filas
    for (size_t i = 0; i < lamina->rows; i++) {
        // bucle para recorrer las columnas
        for (size_t j = 0; j < lamina->columns; j++) {
            // calculamos el indice
            size_t index = i * lamina->columns + j;
            // se intenta leer el valor indicado en el binario, si falla
            // sale del bucle y retorna EXIT_FAILURE
            if (fread(&lamina->temperatures[index], sizeof(double), 1,
            fileobj->file) != 1) {
                error_manager(lamina, fileobj, "Error: Failed to read"
                    "temperature values from file");
                return EXIT_FAILURE;
            }
        }
    }
    // imprimimos la lamina inicial
    print_lamina(lamina);
    return EXIT_SUCCESS;
}
void plan_thread_distribution(Lamina *lamina,
    public_data_t* public_data) {
    // variable con la cantidad de celdas de la lamin
    double total_cells = lamina->rows * lamina->columns;
    // en caso de haber menos de 100 celdas, no vale la pena crear n hilos
    // para realizar el trabajo de la matriz debido al overhead
    if (total_cells < 500) {
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
    // calculamos cuantas celdas debera procesar cada hilo
    size_t cells_per_thread = total_cells / public_data->thread_count;
    // si a cada hilo le tocan menos de 100 celdas, asigna 100 a cada thread y
    // cambia la cantidad de hilos a utilizar
    if (cells_per_thread < 500) {
        cells_per_thread = 500;
        public_data->thread_count = total_cells / 500;
    }
    // calculamos el incremento en filas y columnas que debe hacer cada hilo
    public_data->x_increment = lamina->rows/public_data->thread_count;
    public_data->y_increment = lamina->columns/public_data->thread_count;
}

int starThreads(Lamina *lamina, file_struct *fileobj,
    public_data_t* public_data) {
    // asignamos lamina como dato compartido entre los hilos
    public_data->lamina = lamina;
    public_data->total_cells = lamina->rows * lamina->columns;
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
    // inicializamos next_row para la primera iteracion de update_lamina
    public_data->next_row = 0;
    // inicializamos en 0 la cantidad de estados para evitar calculos erroneos
    public_data->estados = 0;
    // inicializamos en 0 el offset de la matriz actual
    public_data->current_offset = 0;
    // inicializamos en total_cells el offset de la matriz futura
    public_data->next_offset = public_data->total_cells;
    // inicializamos el array de structs privados
    public_data->private_data_array =(void*) private_data;
        // bucle para la creacion de los hilos
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
    free(private_data);
    printf("\n\n");
    printf("Estado: %zu\n\n", public_data->estados);
    // guardamos la cantidad de estados transcurridos
    lamina->k = public_data->estados;
    // guardamos la cantidad de tiempo que le tomo a la simulacion finalizar
    lamina->time *= lamina->k;
    int return_value = 0;
    // si la finalizacio de la simulacion falla, retorna EXIT_FAILURE
    return_value = finish_simulation(lamina, fileobj);
    if (return_value == EXIT_FAILURE) {
        printf("[ERROR] finish_simulation falló\n");
        return EXIT_FAILURE;
    }
    printf("Lamina estabilizada: \n");
    // imprimimos la lamina final
    print_lamina(lamina);
    return EXIT_SUCCESS;
}
// funcion con mapeo estatico

void* update_lamina(void *data) {
    assert(data);
    private_data_t* private_data = (private_data_t*) data;
    public_data_t* public_data = private_data->public_data;
    Lamina* lamina = public_data->lamina;
    size_t thread_count = public_data->thread_count;

    while (1) {
        // Cada hilo actualiza su bloque y guarda unstable_cells localmente
        update_lamina_block(lamina, private_data,
                           public_data->current_offset,
                           public_data->next_offset);

        // Primera barrera: sincronizar fin de trabajo
        int is_serial = pthread_barrier_wait(&public_data->barrier);

        if (is_serial == PTHREAD_BARRIER_SERIAL_THREAD) {
            public_data->estados++;
            size_t total_unstable = 0;
            for (size_t i = 0; i < thread_count; i++) {
                total_unstable += ((private_data_t*)public_data->
                private_data_array)[i].unstable_cells;
            }

            public_data->unstable_blocks = total_unstable;

            // Swap offsets
            size_t temp = public_data->current_offset;
            public_data->current_offset = public_data->next_offset;
            public_data->next_offset = temp;
        }

        // Segunda barrera: asegurar que todos ven updated unstable_blocks y
        // offsets
        pthread_barrier_wait(&public_data->barrier);

        // Condición de parada evaluada sin mutex (lectura simple)
        if (public_data->unstable_blocks == 0) {
            break;
        }
    }

    return NULL;
}

// funcion con mapeo dinamico
/*
oid* update_lamina(void *data) {
    assert(data);
    // para escribir menos, sacamos los datos del private a variables locales
    private_data_t* private_data = (private_data_t*) data;
    public_data_t* public_data = private_data->public_data;
    Lamina* lamina = public_data->lamina;
    size_t thread_count = public_data->thread_count;
    size_t rows = lamina->rows;
    size_t cols = lamina->columns;
    int tid = private_data->thread_num;
    // bucle que termina solo cuando ocurre un error o cuando la lamina no tiene
    // ni una sola celda inestable
    while (1) {
        // reseteamos la cantidad de celdas inestables para cada estado
        // procesado de la matriz
        private_data->unstable_cells = 0;
        // reseteamos la fila actual de cada hilo
        size_t row = 0;
        // barrera para asegurarnos de que ningun hilo empiece a procesar la
        // lamina sin haber reseteado next_row
        if (pthread_barrier_wait(&public_data->barrier) ==
            PTHREAD_BARRIER_SERIAL_THREAD) {
            public_data->next_row = 0;
        }
        pthread_barrier_wait(&public_data->barrier);
        // bucle que finaliza una vez se han iterado todas las filas
        while (row < rows) {
            // un solo hilo accede a la fila que debe ser procesada y aumenta
            // el valor de next_row asegurando que ningun hilo procesara
            // la misma fila que otro
            pthread_mutex_lock(&public_data->row_mutex);
            row = public_data->next_row;
            public_data->next_row++;
            pthread_mutex_unlock(&public_data->row_mutex);
            // condicional por si algun hilo toma un valor de fila que se salga
            // del tamano de la lamina
            if (row < rows) {
                // for para iterar por las columnas
                for (size_t j = 0; j < cols; j++) {
                    // calculamos index para calcular current_index y next_index
                    private_data->index = row * lamina->columns + j;
                    private_data->current_index = public_data->current_offset +
                    private_data->index;
                    private_data->next_index = public_data->next_offset +
                    private_data->index;
                    // si la celda actual es el borde, mantenemos la temperatura
                    if (row == 0 || row == private_data->public_data->
                            lamina->rows - 1 || j == 0 || j == private_data->
                            public_data->lamina->columns - 1) {
                        lamina->temperatures[private_data->next_index] =
                        lamina->temperatures[private_data->current_index];
                        continue;
                    }
                    // reseteamos la temperatura futura de la celda para evitar
                    // calculos erroneos
                    lamina->temperatures[private_data->next_index] = 0;
                    // procesamos la celda actual
                        update_cell(lamina, row, j, lamina->temperatures +
                            public_data->current_offset,
                    lamina->temperatures + public_data->next_offset,
                        &private_data->unstable_cells);
                }
            }
        }
        // barrera para evitar indeterminismo con respecto al conteo de celdas
        // y bloques inestables, lo que provocaria resultados incorrectos
        if (pthread_barrier_wait(&public_data->barrier) ==
                PTHREAD_BARRIER_SERIAL_THREAD) {
            size_t total_unstable = 0;
            for (size_t i = 0; i < thread_count; i++) {
                total_unstable += ((private_data_t*)public_data->
                    private_data_array)[i].unstable_cells;
            }

            public_data->unstable_blocks = total_unstable;
            public_data->estados++;

            size_t tmp = public_data->current_offset;
            public_data->current_offset = public_data->next_offset;
            public_data->next_offset = tmp;
        }
        // barrera para esperar a que se hayan contado todos los bloques de
        // todos los hilos, evitando indeterminismo para la condicion de parada
        pthread_barrier_wait(&public_data->barrier);
        // si no existen bloques inestables, no existen celdas inestables
        // por ende, se ha estabilizado la lamina
        if (public_data->unstable_blocks == 0) {
            break;
        }
    }

    return NULL;
}
*/


void update_lamina_block(Lamina* lamina, private_data_t* private_data,
    size_t current_offset,
    size_t next_offset) {
    private_data->unstable_cells = 0;
    size_t x_increment = private_data->public_data->x_increment;
    // Dividir solo por filas (x), no por columnas
    private_data->x1 = private_data->thread_num * x_increment;
    private_data->x2 = (private_data->thread_num ==
        private_data->public_data->thread_count - 1)
        ? private_data->public_data->lamina->rows
        : (private_data->thread_num + 1) * x_increment;
    private_data->y1 = 0;
    private_data->y2 = private_data->public_data->lamina->columns;
    // recorrido del bloque correspondiente de cada hilo, que ignora los bordes
    // pues estos son constantes
    for (size_t i = private_data->x1; i < private_data->x2; i++) {
        for (size_t j = private_data->y1; j < private_data->y2; j++) {
            // calculamos index para calcular current_index y next_index
            private_data->index = i * lamina->columns + j;
            private_data->current_index = current_offset + private_data->index;
            private_data->next_index = next_offset + private_data->index;
            // si la celda actual es el borde, mantenemos la temperatura
            if (i == 0 || i == private_data->public_data->lamina->rows - 1 ||
                j == 0 || j == private_data->public_data->lamina->columns - 1) {
                lamina->temperatures[private_data->next_index] =
                lamina->temperatures[private_data->current_index];
                continue;
            }
            // reseteamos la temperatura futura de la celda para evitar
            // calculos erroneos
            lamina->temperatures[private_data->next_index] = 0;
                update_cell(lamina, i, j, lamina->temperatures + current_offset,
            lamina->temperatures + next_offset, &private_data->unstable_cells);
        }
    }
    return;
}
void update_cell(Lamina *lamina, size_t row, size_t column, double *current_mat,
    double *next_mat, size_t* unstable_cells) {
    double diff = 0;
    size_t cols = lamina->columns;
    size_t idx = row * cols + column;
    // guardamos las temperaturas de las celdas adyacentes
    double up = current_mat[(row - 1) * cols + column];
    double down = current_mat[(row + 1) * cols + column];
    double left = current_mat[row * cols + (column - 1)];
    double right = current_mat[row * cols + (column + 1)];
    // calculamos la temperatura futura de la celda
    next_mat[idx] = current_mat[idx] +
        ((lamina->time * lamina->conductivity) / (lamina->height *
            lamina->height)) *
        (up + down + right + left - 4 * current_mat[idx]);
    // verificamos si la celda esta estabilizada, si no lo esta, aumentamos el
    // valor de unstable_cells
    diff = fabs(next_mat[idx] - current_mat[idx]);
        if (diff > lamina->epsilon) {
            (*unstable_cells)++;
        }
}
int finish_simulation(Lamina * lamina, file_struct *fileobj) {
    // si falla el manejo del archivo de reporte, retorna EXIT_FAILURE
    int return_value = report_file(lamina, fileobj);
    if (return_value == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
void print_lamina(Lamina* lamina) {
    return;
    printf("Matriz de temperaturas:\n");
    size_t index = 0;
    // bucle para recorrer las filas
    for (uint64_t i = 0; i < lamina->rows; i++) {
    // bucle para recorrer las columnas
        for (uint64_t j = 0; j < lamina->columns; j++) {
            index = i * lamina->columns + j;
            printf("[%g] ", lamina->temperatures[index]);
        }
        printf("\n");
    }
}
void error_manager(Lamina *lamina, file_struct *fileobj,
    const char* error_message) {
    // imprimimos el mensaje de error recibido
    fprintf(stderr, "%s\n", error_message);
    // llamamos a liberar toda la memoria reservada
    delete_lamina(lamina, fileobj);
}
void delete_lamina(Lamina * lamina, file_struct* fileobj) {
    // si existe el objeto lamina, libera la memoria de sus atributos
    if (lamina) {
        // si existe la lamina, la libera
        if (lamina->temperatures) {
            free(lamina->temperatures);
        }
        // llama al destructor de fileobj
        delete_files(fileobj);
        // libera la lamina
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
