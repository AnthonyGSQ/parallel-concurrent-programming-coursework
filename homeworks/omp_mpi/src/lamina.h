// Copyright 2025 Anthony Sanchez
#ifndef HOMEWORKS_SERIAL_SRC_LAMINA_H_
#define HOMEWORKS_SERIAL_SRC_LAMINA_H_
#define _POSIX_C_SOURCE 200112L
#include "FileManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mpi.h>
#include <omp.h>
#include <time.h>

/**
 * @struct Lamina
 * @brief Estructura que representa una lámina térmica en la simulación.
 * 
 * Esta estructura contiene los parámetros y datos necesarios para realizar
 * la simulación térmica de una lámina utilizando el método de diferencias
 * finitas.
 * Incluye las dimensiones de la lámina, las temperaturas en la misma y los
 * parámetros a utilizar
 */ 
typedef struct Lamina {
    /**
     * @brief cantidad de filas de la lamina
     */
    uint64_t rows;
    /**
     * @brief cantidad de columnas de la lamina
     */
    uint64_t columns;
    /**
     * @brief El valor de k utilizado en la simulación.
     */
    double k;
    /**
     * tiempo transcurrido en la simulacion
     */
    double time;
    /**
     * @brief Conductividad térmica del material.
     */
    double conductivity;
    /**
     * @brief Altura de la lámina.
     */
    double height;
    /**
     * @brief Epsilon utilizado para la estabilidad de la simulación.
     * 
     * Este valor define el umbral de estabilidad para detener la simulación
     * cuando las temperaturas ya no cambian significativamente entre pasos.
     */
    double thread_count;
    /**
     * @brief parametro usado para verificar si una celda esta estabilizada
     */
    double epsilon;
    /**
     * @brief matriz de temperaturas
     */
    double *temperatures;
} Lamina;
/**
 * @brief Estructura de datos compartidos entre los hilos.
 *
 * Contiene información común utilizada por todos los hilos, como cuántas filas
 * o columnas
 * debe procesar cada uno, el número total de hilos, y un puntero a la lámina
 * con las matrices de temperatura.
 */
typedef struct {
    size_t x_increment;   /**< Cantidad de columnas a procesar por hilo */
    size_t y_increment;   /**< Cantidad de filas que debe procesar cada hilo. */
    size_t thread_count;  /**< Número total de hilos en ejecución. */
    Lamina* lamina;       /**< Puntero a la estructura lamina */
    size_t estados; /* cantidad de estados transcurridos*/
    size_t unstable_blocks; /*cantidad de bloques de la matriz inestables*/
    size_t total_cells; /** cantidad de celdas de la lamina */
    size_t current_offset; /** offset de las temperaturas actuales */
    size_t next_offset; /** offset de las temperaturas futuras*/
    size_t next_row; /** contador de la fila actual a procesar */
    void* private_data_array; /** array de structs privados de los hilos */
} public_data_t;
/**
 * @brief Estructura de datos privados de cada hilo.
 *
 * Contiene los límites del área que un hilo debe procesar, su identificador y
 * un contador local de celdas inestables. También incluye un puntero a los
 * datos públicos.
 */
typedef struct {
    size_t x1;                   /**< Límite inferior en X del área asignada. */
    size_t x2;                   /**< Límite superior en X del área asignada. */
    size_t y1;                   /**< Límite inferior en Y del área asignada. */
    size_t y2;                   /**< Límite superior en Y del área asignada. */
    size_t thread_num;           /**< Número o identificador del hilo. */
    size_t unstable_cells;       /**< Numero de celdas inestables por bloque */
    size_t index;                /** indice de la lamina */
    size_t current_index;        /** indice de las temperaturas actuales */
    size_t next_index;           /** indice de las temperaturas futuras */
    public_data_t* public_data;  /**< Puntero a la estructura de datos publica*/
} private_data_t;
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
 * abrir, la función imprime un mensaje de error y retorna `EXIT_FAILURE`.
 */

int lamina_constructor(int argc, char *argv[]);
/**
 * @brief Determina la cantidad de hilos a utilizar en función de los argumentos
 * de entrada o el sistema.
 * 
 * @param argc Número de argumentos proporcionados al programa.
 * @param argv Lista de argumentos proporcionados al programa.
 * @return size_t Cantidad de hilos a utilizar.
 */
size_t determine_thread_count(int argc, char *argv[]);
/**
 * @brief Lee el archivo de entrada y procesa cada línea.
 * 
 * @param file_path Ruta del archivo a leer.
 * @param thread_count Cantidad de hilos que se usarán en el procesamiento.
 * @return int EXIT_SUCCESS si la lectura fue exitosa, EXIT_FAILURE en caso de
 * error.
 */
int read_input_file(const char *file_path, size_t thread_count);
/**
 * @brief Inicializa las estructuras necesarias para procesar una línea.
 * 
 * @param file_path Ruta del archivo .txt de entrada.
 * @param thread_count Cantidad de hilos que se usarán en el procesamiento.
 * @param lamina Puntero a la estructura Lamina que se inicializará.
 * @param fileobj Puntero a la estructura file_struct que se inicializará.
 * @param public_data Puntero a los datos compartidos entre hilos
 * @return int EXIT_SUCCESS si la inicialización fue exitosa, EXIT_FAILURE en
 * caso de error.
 */
int initialize_resources(const char *file_path, size_t thread_count,
    Lamina **lamina, file_struct **fileobj, public_data_t **public_data);
/**
 * @brief funcion encargada de asignar una lamina a cada proceso de MPI
 * @param file_path archivo txt del cual se extrae la linea a procesar
 * @param thread_count cantidad de hilos que usara cada proceso
 */
void distributionOfLines(const char *file_path, size_t thread_count);
/**
 * @brief Procesa una lámina abriendo el archivo binario y ejecutando la
 * simulación.
 * 
 * @param lamina Puntero a la estructura Lamina.
 * @param fileobj Puntero a la estructura file_struct.
 * @param public_data Puntero a la estructura public_data_t.
 * @return int EXIT_SUCCESS si el proceso fue exitoso, EXIT_FAILURE en caso de
 * error.
 */
int process_lamina(Lamina *lamina, file_struct *fileobj,
    public_data_t *public_data);
/**
 * @brief Procesa una línea del archivo de entrada, inicializando recursos y ejecutando la simulación.
 * 
 * @param line Línea del archivo de entrada a procesar.
 * @param thread_count Cantidad de hilos a usar.
 * @param file_path Ruta del archivo .txt de entrada.
 */
void process_line(const char *line, size_t thread_count, const char *file_path);
/**
 * @brief Libera public_data y llama a delete_lamina para limpiar los recursos.
 * 
 * @param lamina Puntero a la estructura Lamina.
 * @param fileobj Puntero a la estructura file_struct.
 * @param public_data Puntero a la estructura public_data_t.
 */

void clean_resources(Lamina *lamina, file_struct *fileobj,
    public_data_t *public_data);
/**
 * @brief Lee y valida los parámetros de la lamina
 * 
 * Extrae los valores tiempo, conductividad, height y epsilon, guardandolos en
 * el struct lamina
 * 
 * @param lamina Puntero a la estructura Lamina que tiene todos los parametros
 * @param fileobj Puntero a la estructura encargada del manejo de archivos
 * @param line Línea de texto con los parámetros a leer.
 * 
 * @return EXIT_SUCCESS si encuentra los parametros, EXIT_FAILURE si no
 * 
 * @note Si la línea no tiene los parámetros esperados o alguno de los valores
 * es inválido, 
 *       la función imprime un mensaje de error y retorna `EXIT_FAILURE`.
 */
int reading_parameters(Lamina *lamina, file_struct* fileobj, char* line);
/**
 * @brief Reserva memoria para las matrices de temperaturas y lee el archivo
 * binario.
 * @param lamina Estructura Lamina donde se almacenarán los datos.
 * @param fileobj Puntero a la estructura encargada del manejo de archivos
 * @param public_data Puntero a los datos compartidos entre los hilos
 * 
 * @return `EXIT_SUCCESS` si la memoria se reserva correctamente,
 * `EXIT_FAILURE` en caso de error.
 * @note Si la lectura del archivo falla o la reserva de memoria no es exitosa, 
 * la función imprime un mensaje de error y retorna `EXIT_FAILURE`.
 */
int create_lamina(Lamina *lamina, file_struct* fileobj, public_data_t
    *public_data);
/**
 * @brief Funcion a cargo de llenar la matriz con los valores del binario
 * 
 * @param lamina Puntero al objeto lamina actual
 * @param fileobj Puntero al objeto fileobj que contiene todos los datos de los
 * archivos binarios y tsv's
 * @return retorna EXIT_FAILURE si nada falla, en caso de no poder leer el
 * valor del binario, retorna EXIT_FAILURE
 */
int fillMatriz(Lamina * lamina, file_struct* fileobj);
/**
 * @brief Funcion que calcula el tamano del bloque que procesara cada hilo
 * 
 * En caso de recibir demaciados hilos, los limita, si la lamina recibida
 * tiene menos de 100 celdas, limita la cantidad de hilos a utilizar a solo uno
 * , esto debido al overhead ya que no vale la pena crear n hilos para 100
 * celdas o menos
 * @param lamina Puntero al objeto lamina actual
 * @param public_data estructura compartida entre hilos
 */
void plan_thread_distribution(Lamina *lamina,
    public_data_t* public_data);
/**
 * @brief Funcion a cargo de actualizar la lamina de estado k a estado k+1
 * @param lamina Puntero al objeto lamina actual
 * @param fileobj Puntero al objeto encargado del manejo de archivos
 * @param public_data Puntero a los datos compartidos entre hilos
 * @return EXIT_FAILURE si se presenta algun fallo, EXIT_SUCCESS si no.
 */
int updateWithOmp(Lamina *lamina, file_struct *fileobj,
    public_data_t* public_data);

/**
 * @brief Funcion que actualiza el bloque de la matriz correspondiente a cada
 * hilo
 * La funcion primeramente calcula los puntos x1, x2, y1, y2 para saber
 * de donde a donde debe procesar cada hilo la matriz. Cabe resaltar que el
 * x2, y2 son excluyentes, de manera en que nunca existen dos hilos que
 * modifiquen la misma celda, evitando condiciones de carrera o similar
 * 
 * @param lamina puntero al objeto lamina
 * @param private_data Puntero al struct privado de cada hilo
 * @param current_offset posicion donde se encuentran las temperaturas actuales
 * @param next_offset posicion donde se encuentran las temperaturas futuras
 */
void update_lamina_block(Lamina* lamina, private_data_t* private_data,
    size_t current_offset,
    size_t next_offset);
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
 * @param current_mat matriz de temperaturas actuales
 * @param next_mat matriz de las temperaturas futuras
 * @param unstable_cells Puntero a la cantidad de celdas no estables del bloque
 * 
 * @note Solo se actualizan las celdas internas de la matriz, evitando los
 * bordes.
 */
void update_cell(Lamina *lamina, size_t row, size_t column, double *current_mat,
    double *next_mat, size_t* unstable_cells);
/**
 * @brief Finaliza la simulación y llama a report_file
 * 
 * Llama a `report_file` para escribir el resultado de la simulación. 
 * Si la generación del reporte falla, retorna `EXIT_FAILURE`.
 * 
 * @param lamina Puntero a la estructura lamina
 * @param fileobj PUntero a la estructura de archivos
 * 
 * @return `EXIT_SUCCESS` si genero el reporte, EXIT_FAILURE si no
 */
int finish_simulation(Lamina *lamina, file_struct *fileobj);

/**
 * @brief Imprime la matriz de temperaturas en la consola.
 * 
 * @param lamina Estructura Lamina con las temperaturas a imprimir.
 */
void print_lamina(Lamina *lamina);
/**
 * @brief Maneja casi todos los errores, llamando a delete_lamina luego
 *
 * Esta funcion recibe el mensaje a imprimir como reporte del error, una vez
 * hecho eso, llama la funcion delete_lamina() para liberar toda la memoria
 * aun no liberada
 *
 * @param lamina Un puntero a la estructura lamina
 * @param fileobj Puntero a la estructura encargada del manejo de archivos
 * @param error_message Un mensaje de error que se imprime en la salida de
 * error estándar.
 */
void error_manager(Lamina *lamina, file_struct* fileobj,
    const char* error_message);
/**
 * @brief Libera los recursos de lamina
 *
 * Esta función libera la memoria dinámica utilizada por la estructura `Lamina`
 * Antes de hacerle free verifica que exista lo que se piensa borrar para evitar
 * intentar liberar memoria ya liberada o que nunca fue reservada.
 *
 * @param lamina Un puntero a la estructura lamina, si no existe, no hace nada.
 * @param fileobj Puntero a la estructura encargada del manejo de archivos
 */
void delete_lamina(Lamina *lamina, file_struct *fileobj);
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
void format_time(const time_t seconds, char* text, const size_t capacity);
#endif  // HOMEWORKS_SERIAL_SRC_LAMINA_H_
