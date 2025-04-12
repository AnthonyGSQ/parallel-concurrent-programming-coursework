// Copyright 2025 Anthony Sanchez
#ifndef HOMEWORKS_SERIAL_SRC_LAMINA_H_
#define HOMEWORKS_SERIAL_SRC_LAMINA_H_
#include "FileManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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
    uint64_t k;
    uint64_t time;
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
    public_data_t *public_data;
    double **temperatures;
    double **next_temperatures;
} Lamina;

typedef struct {
    double x1;
    double x2;
    double y1;
    double y2;
} private_data_t;

typedef struct {
    size_t x_increment;
    size_t y_increment;
    private_data_t *private_data;
} public_data_t;

/**
 * @brief Construye el objeto Lamina a partir de los parámetros dados.

 * Esta función valida los parámetros, abre los archivos necesarios y 
 * crea el objeto Lamina. Retorna EXIT_FAILURE si algo sale mal.
 * 
 * @param argc Número de argumentos.
 * @param argv Arreglo de argumentos.
 * 
 * @return EXIT_SUCCESS o EXIT_FAILURE.
 */
int lamina_constructor(int argc, char *argv[]);
/**
 * @brief Verifica que los parámetros recibidos desde el archivo sean válidos.
 * 
 * Esta función lee los parámetros del txt
 * @param lamina Puntero a la estructura Lamina
 * @param filename Nombre del archivo de entrada.
 * @param line Línea leída desde el txt que contiene los parametros
 * 
 * @return `EXIT_SUCCESS` si los parámetros son válidos, `EXIT_FAILURE` si hay
 * un error.
 */
int reading_parameters(Lamina *lamina, file_struct* fileobj, char* line);
/**
 * @brief Reserva memoria para las matrices de temperaturas y lee el archivo binario.
 * 
 * @param lamina Estructura Lamina donde se almacenarán los datos.
 * 
 * @return `EXIT_SUCCESS` si la memoria se reserva correctamente, `EXIT_FAILURE` en caso de error.
 */
int create_lamina(Lamina *lamina, file_struct* fileobj);
/***
 * @brief Rellena la matriz con los valores del archivo binario
 * 
 * @param lamina Estructura lamina con los datos de la simulacion
 * 
 * @return EXIT_SUCCESS en caso de poder rellenar toda la matriz con los valores
 *  del binario, EXIT_FAILURE en caso contrario
 */
int fillMatriz(Lamina * lamina, file_struct* fileobj);
/***
 * @brief Funcion encargada de calcular cuanto trabajo debe realizar cada hilo
 * , es decir, calcula el tamano del bloque a procesar para los hilos de manera
 *  en que cada hilo hara una cantidad similar de trabajo.abort
 * @param lamina Puntero al struct lamina actual
 * @param fileobj Puntero al struct del manejo de archivos
 */
void plan_thread_distribution(Lamina *lamina, file_struct * fileobj);
/**
 * @brief Actualiza la lamina del estado k al estado k+1.
 * 
 * @param lamina Estructura Lamina que contiene las matrices de temperaturas.
 * 
 * @return `EXIT_SUCCESS` si la actualización es exitosa.
 */
int update_lamina(Lamina *lamina, file_struct *fileobj);
/***
 * @brief Actualiza el bloque i de la matriz, a cargo del hilo i
 * @param lamina puntero a la lamina actual
 * @param fileobj puntero al struct encargado del manejo de archivos
 * @param private_data_t puntero a los datos privados de cada hilo
 */
void* update_lamina_block(void *data);
/**
 * @brief Actualiza una celda específica en la lamina usando la fórmula de
 * difusión de calor.
 * 
 * @param lamina Estructura Lamina con las temperaturas actuales.
 * @param row Fila de la celda a actualizar.
 * @param column Columna de la celda a actualizar.
 */
int update_cell(Lamina *lamina, uint64_t row, uint64_t column);

/**
 * @brief Finaliza la simulación y guarda los resultados en archivos.
 * 
 * @param lamina Estructura Lamina con los datos finales.
 * 
 * @return `EXIT_SUCCESS` si la simulación se completa exitosamente.
 */
int finish_simulation(Lamina *lamina, file_struct *fileobj);

/**
 * @brief Imprime la matriz de temperaturas en la consola.
 * 
 * @param lamina Estructura Lamina con las temperaturas a imprimir.
 */
void print_lamina(Lamina *lamina);

/**
 * @brief Muestra un mensaje de error y libera la memoria de la lamina.
 * 
 * @param lamina Estructura Lamina que se va a liberar.
 * @param error_message Mensaje de error a mostrar.
 */
void error_manager(Lamina *lamina, file_struct* fileobj, const char* error_message);

/**
 * @brief Libera toda la memoria ocupada por la lamina.
 * 
 * @param lamina Estructura Lamina cuya memoria se va a liberar.
 */
void delete_lamina(Lamina *lamina, file_struct *fileobj);

/**
 * @brief Convierte el tiempo en segundos a un formato legible (horas/días/meses).
 * 
 * @param seconds Tiempo en segundos.
 * @param text Cadena donde se almacenará el tiempo formateado.
 * @param capacity Capacidad máxima del buffer `text`.
 */
void format_time(const time_t seconds, char* text, const size_t capacity);
#endif  // HOMEWORKS_SERIAL_SRC_LAMINA_H_
