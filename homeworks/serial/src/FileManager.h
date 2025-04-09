// Copyright 2025 Anthony Sanchez
#ifndef HOMEWORKS_SERIAL_SRC_FILEMANAGER_H_
#define HOMEWORKS_SERIAL_SRC_FILEMANAGER_H_
#include "lamina.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
typedef struct {
    char base_route[256];
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