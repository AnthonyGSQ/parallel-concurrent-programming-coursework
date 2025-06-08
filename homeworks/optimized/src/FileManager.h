// Copyright 2025 Anthony Sanchez
#ifndef HOMEWORKS_SERIAL_SRC_FILEMANAGER_H_
#define HOMEWORKS_SERIAL_SRC_FILEMANAGER_H_
#include "lamina.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
/***
 * @struct shared_file_data
 * @brief Estructura encargada del manejo de archivos, tanto las rutas
 * dadas por el usuario, como el manejo del archivo binario y el archivo
 * binario sobre el cual trabaja la simulacion de calor
 */
typedef struct {
    /***
     * @brief variable utilizada para guardar la ruta base y poder reutilizarla
     * para la construccion de rutas para encontrar el archivo binario y 
     * el archivo tsv
     */
    char base_route[256];
    /***
     * @brief variable donde queda guardada la ruta para leer los binarios
     * indicados en el archivo txt recibido
     */
    char binary_file_name[256];
    /**
     * @brief Archivo del binario que contiene los datos de la matriz
     */
    FILE* file;
    /**
     * @brief archivo tsv donde se guardaran los reportes de cada lamina
     */
    FILE* report_file;
} file_struct;
typedef struct Lamina Lamina;
/**
 * @brief Genera y guarda un archivo de reporte en formato TSV con la
 * información de la simulación de la lámina.
 *
 * Esta función crea o abre un archivo TSV donde se almacenan los datos de la
 * simulación, vease el nombre del binario, tiempo transcurrido, epsilon,
 * height y conductivity Si no se puede abrir el archivo, se crea uno nuevo en
 * una ruta alternativa (dentro de la carpeta job###).
 * Además, se formatea el tiempo en formato legible para una persona
 * 
 * @param lamina Un puntero a la estructura lamina
 * @param file_obj Puntero a la estructura encargada del manejo de archivos
 *
 * @return EXIT_SUCCESS Si el reporte fue exitoso, EXIT_FAILURE si no
 *
 * @note Esta función intenta abrir un archivo en la ruta proporcionada por la
 * estructura `Lamina`. Si no se puede abrir, intentará crear un archivo en una
 * ruta alternativa.
 */
int report_file(Lamina *lamina, file_struct *file_obj);
/**
 * @brief Libera los recursos asociados a los archivos utilizados.
 *
 * Cierra los archivos abiertos en la estructura `file_struct` y libera
 * la memoria asignada a dicha estructura.
 *
 * @param fileobj Puntero a la estructura que contiene los archivos abiertos.
 * @return EXIT_SUCCESS si la operación se realiza correctamente.
 */
int delete_files(file_struct *fileobj);
#endif  // HOMEWORKS_PTHREAD_SRC_FILEMANAGER_H_
