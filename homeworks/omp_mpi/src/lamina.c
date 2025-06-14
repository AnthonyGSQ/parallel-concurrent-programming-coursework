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
    if (argc < 2) {
        error_manager(NULL, NULL,
            "Invalid entrance: you need to specify a .txt and"
            " the amounth of threads you want to use");
        return EXIT_FAILURE;
    }

    size_t thread_count = determine_thread_count(argc, argv);

    // Lectura del archivo de entrada
    if (read_input_file(argv[1], thread_count) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
size_t determine_thread_count(int argc, char *argv[]) {
    int64_t thread_count = 0;
    // si nos dan un tercer parametro vemos si es una cantidad valida de hilos
    // si no lo es, tomamos thread_count como la cantidad de hilos del sistema
    if (argc == 3) {
        char *end;
        thread_count = strtoul(argv[2], &end, 10);
        if (*end != '\0' || thread_count <= 0) {
            thread_count = sysconf(_SC_NPROCESSORS_CONF);
            fprintf(stderr,
                "Warning!: invalid thread_count, taking %" PRId64 " as "
                "thread_number\n", thread_count);
        }
    } else {
        thread_count = sysconf(_SC_NPROCESSORS_CONF);
    }

    return (size_t)thread_count;
}
int read_input_file(const char *file_path, size_t thread_count) {
    FILE *file = fopen(file_path, "r");
    // si el archivo no se abre correctamente, mostramos un mensaje de error
    if (!file) {
        fprintf(stderr, "Error: could not open %s file\n", file_path);
        return EXIT_FAILURE;
    }

    char line[256] = "";
    // llamamos a la funcion que distribuye las lineas del txt entre los
    // procesos MPI
    distributionOfLines(file_path, thread_count);
    return EXIT_SUCCESS;
}
void distributionOfLines(const char *file_path, size_t thread_count) {
    // TODO(AnthonyGSQ): como recibo el rank y size?
    int rank, size;
    // inicializacion de MPI
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    printf("[MPI] Proceso %d de %d inicializado con %" PRIu64" hilos\n", rank,
        size, thread_count);
    // abrimos el archivo txt con las lineas a procesar, si no se pudo abrir
    // se muestra un mensaje de error y se aborta el proceso MPI
    FILE *file = fopen(file_path, "r");
    if (!file) {
        fprintf(stderr, " Error: No se pudo abrir el archivo en el"
            "proceso %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Contar el número total de líneas en el archivo (solo el proceso 0)
    int total_lines = 0;
    // solo el proceso 0 cuenta las lineas del archivo
    if (rank == 0) {
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), file)) {
            total_lines++;
        }
        rewind(file);
        printf("[MPI] Proceso 0 detectó %d líneas en el archivo\n",
            total_lines);
    }

    // Broadcast del número total de líneas a todos los procesos
    MPI_Bcast(&total_lines, 1, MPI_INT, 0, MPI_COMM_WORLD);
    printf("[MPI] Proceso %d recibió total_lines = %d\n", rank, total_lines);
    // cantidad de lineas que le corresponde a cada proceso
    int lines_per_proc = total_lines / size;
    // primera linea que le toca al proceso
    int start_line = rank * lines_per_proc;
    // linea final que le toca a ese proceso
    int end_line = (rank == size - 1) ? total_lines : (start_line +
        lines_per_proc);

    // Mover el puntero del archivo al inicio de las líneas asignadas
    char buffer[1024];
    for (int i = 0; i < start_line; i++) {
        fgets(buffer, sizeof(buffer), file);
    }

    printf("[MPI] Proceso %d procesará líneas %d a %d\n", rank, start_line,
        end_line);

    // Procesar las líneas asignadas a este proceso
    for (int i = start_line; i < end_line; i++) {
        if (fgets(buffer, sizeof(buffer), file)) {
            printf("[MPI] Proceso %d leyendo línea %d: %s", rank, i, buffer);
            process_line(buffer, thread_count, file_path);
        }
    }
    // cerramos el txt
    fclose(file);
    printf("[MPI] Proceso %d finalizó\n", rank);
    // barrera para evitar que se termine MPI antes de que todos los procesos
    // terminen
    MPI_Barrier(MPI_COMM_WORLD);
    // terminamos MPI
    MPI_Finalize();
}
void process_line(const char *line, size_t thread_count,
    const char *file_path) {
    Lamina *lamina;
    file_struct *fileobj;
    public_data_t *public_data;

    // Inicializar estructuras
    if (initialize_resources(file_path, thread_count, &lamina, &fileobj,
        &public_data) == EXIT_FAILURE) {
        return;
    }

    // Leer parámetros y procesar la lámina
    if (reading_parameters(lamina, fileobj, line) == EXIT_FAILURE) {
        clean_resources(lamina, fileobj, public_data);
        return;
    }

    if (process_lamina(lamina, fileobj, public_data) == EXIT_FAILURE) {
        clean_resources(lamina, fileobj, public_data);
        return;
    }

    // Liberar recursos
    clean_resources(lamina, fileobj, public_data);
}
int initialize_resources(const char *file_path, size_t thread_count,
    Lamina **lamina, file_struct **fileobj, public_data_t **public_data) {
    *lamina = (Lamina *)calloc(1, sizeof(Lamina));
    *fileobj = (file_struct *)calloc(1, sizeof(file_struct));
    *public_data = (public_data_t *)calloc(1, sizeof(public_data_t));

    if (!(*lamina) || !(*fileobj) || !(*public_data)) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return EXIT_FAILURE;
    }

    (*public_data)->thread_count = thread_count;
    snprintf((*fileobj)->base_route, sizeof((*fileobj)->base_route), "%s",
        file_path);

    char *lastSlash = strrchr((*fileobj)->base_route, '/');
    if (!lastSlash) {
        fprintf(stderr, "Error: invalid route %s\n", file_path);
        error_manager(*lamina, *fileobj,
            "Error: invalid route, please check the file path");
        return EXIT_FAILURE;
    }
    *(lastSlash + 1) = '\0';

    return EXIT_SUCCESS;
}
int process_lamina(Lamina *lamina, file_struct *fileobj,
    public_data_t *public_data) {
    if (!lamina || !fileobj || !public_data) {
        fprintf(stderr, "Error: Invalid input to process_lamina\n");
        return EXIT_FAILURE;
    }

    fileobj->file = fopen(fileobj->binary_file_name, "rb");
    if (!fileobj->file) {
        fprintf(stderr, "Error: could not open binary file %s\n",
            fileobj->binary_file_name);
        return EXIT_FAILURE;
    }

    printf("Creating lamina...\n");
    if (create_lamina(lamina, fileobj, public_data) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (updateWithOmp(lamina, fileobj, public_data) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
void clean_resources(Lamina *lamina, file_struct *fileobj,
    public_data_t *public_data) {
    // liberamos toda la memoria que se ha reservado
    free(public_data);
    // llamamos al destructor de todo
    delete_lamina(lamina, fileobj);
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
    // print_lamina(lamina);
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
int updateWithOmp(Lamina *lamina, file_struct *fileobj,
    public_data_t* public_data) {
    public_data->lamina = lamina;
    public_data->total_cells = lamina->rows * lamina->columns;
    size_t thread_count = public_data->thread_count;
    private_data_t* private_data = calloc(thread_count, sizeof(private_data_t));
    if (!private_data) {
        error_manager(lamina, fileobj, "Error: could not make calloc for"
            "private_data");
        return EXIT_FAILURE;
    }

    public_data->unstable_blocks = 1;
    public_data->estados = 0;
    public_data->current_offset = 0;
    public_data->next_offset = public_data->total_cells;

    omp_set_num_threads(thread_count);
    size_t unstable_cells_local = 0;
    #pragma omp parallel
    {
        while (1) {
            // Bucle paralelo: cada hilo actualiza su bloque
            #pragma omp for schedule(dynamic, 1)\
            reduction(+:unstable_cells_local)
                for (size_t i = 0; i < thread_count; i++) {
                    private_data[i].thread_num = i;
                    private_data[i].public_data = public_data;
                    update_lamina_block(lamina, &private_data[i],
                        public_data->current_offset, public_data->next_offset);
                    unstable_cells_local += private_data[i].unstable_cells;
                }
            // barrera para evitar indeterminismo en el conteo de celdas
            // inestables
            #pragma omp barrier
            // Solo un hilo realiza la parte de afuera del bucle paralelo
            #pragma omp single
            {
                public_data->estados++;

                // Swap offsets
                size_t temp = public_data->current_offset;
                public_data->current_offset = public_data->next_offset;
                public_data->next_offset = temp;
                public_data->unstable_blocks = unstable_cells_local;
                unstable_cells_local = 0;
            }

            // Salida de todos los hilos si ya no hay celdas inestables
            #pragma omp barrier
                if (!public_data->unstable_blocks) {
                    break;
                }
        }
    }
    lamina->k = public_data->estados;
    lamina->time *= lamina->k;
    printf("ESTADO: %" PRIu64 "\n", public_data->estados);
    free(private_data);
    if (finish_simulation(lamina, fileobj) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

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
    // TODO(AnthonyGSQ): comentado para probar en el cluster
    // int return_value = report_file(lamina, fileobj);
    // if (return_value == EXIT_FAILURE) {
    //     return EXIT_FAILURE;
    // }
    return EXIT_SUCCESS;
}
void print_lamina(Lamina* lamina) {
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
