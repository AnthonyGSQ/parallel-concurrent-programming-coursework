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
     * @brief Archivo de binario que contiene los datos de la matriz
     */
    FILE* file;
    FILE* report_file;
} shared_file_data;
typedef struct Lamina Lamina;

int report_file(Lamina *lamina, shared_file_data *file_obj);
int delete_files(shared_file_data *fileobj);
#endif