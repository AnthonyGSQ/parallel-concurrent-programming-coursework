// Copyright 2025 Anthony Sanchez
#ifndef HOMEWORKS_SERIAL_SRC_LAMINA_H_
#define HOMEWORKS_SERIAL_SRC_LAMINA_H_
#define _POSIX_C_SOURCE 200112L
#include "FileManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
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
    uint64_t rows;
    uint64_t columns;
    /**
     * @brief El valor de k utilizado en la simulación.
     */
    double k;
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
    double epsilon;
    double **temperatures;
    double **next_temperatures;
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
    // mutex para modificar swap_matrix y unstable_blocks
    pthread_mutex_t can_add_unstable_blocks;
    pthread_barrier_t barrier; /* barrera para sincronizar los hilos*/
    size_t estados; /* cantidad de estados transcurridos*/
    size_t unstable_blocks; /*cantidad de bloques de la matriz inestables*/
    double **temp; /*puntero para el swap de matrices*/
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
 * @brief Funcion encargada de crear y destruir todos los hilos
 * 
 * @param lamina Puntero al struct lamina
 * @param fileobj Puntero al struct encargado del manejo de archivos
 * @param public_data Puntero al struct de datos compartidos entre hilos
 */
int starThreads(Lamina *lamina, file_struct *fileobj,
    public_data_t* public_data);
/**
 * @brief Actualiza la matriz de temperaturas hasta estabilizarla
 * 
 * Recorre la matriz de temperaturas, actualizando cada celda hasta que todas
 * sean estables (que tengan una diferencia entre el futuro estado y el actual
 * menor a epislon). Intercambia las matrices `temperatures` y
 * next_temperatures` en cada iteración.
 * 
 * @param lamina Puntero a la estructura lamina
 * @param fileobj Puntero a la estructura encargada del manejo de archivos
 * @param public_data puntero a la estructura de datos compartidos entre hilos
 * 
 * @return EXIT_SUCCESS si estabilizo la lamina, EXIT_FAILURE si no
 * 
 * @note Si la simulación falla al finalizar, la función devuelve
 * `EXIT_FAILURE`.
 */
/**
 * @brief Funcion encargada de iniciar y destruir todos los threads una sola vez
 * 
 * @param lamina Puntero a la estructura lamina
 * @param fileobj Puntero al struct encargado del manejo de archivos
 * @param public_data Puntero al struct de datos compartidos
 * @return EXIT_SUCCESS si no hubo fallos, EXIT_FAILURE en el caso contrario
 */
void* update_lamina(void* data);
/**
 * @brief Funcion que actualiza el bloque de la matriz correspondiente a cada
 * hilo
 * La funcion primeramente calcula los puntos x1, x2, y1, y2 para saber
 * de donde a donde debe procesar cada hilo la matriz. Cabe resaltar que el
 * x2, y2 son excluyentes, de manera en que nunca existen dos hilos que
 * modifiquen la misma celda, evitando condiciones de carrera o similar
 * 
 * @param private_data Puntero al struct privado de cada hilo
 */
void update_lamina_block(private_data_t* private_data);
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
 * @param unstable_cells Puntero a la cantidad de celdas no estables del bloque
 * 
 * @note Solo se actualizan las celdas internas de la matriz, evitando los
 * bordes.
 */
void update_cell(Lamina *lamina, size_t row, size_t column,
    size_t *unstable_cells);
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
