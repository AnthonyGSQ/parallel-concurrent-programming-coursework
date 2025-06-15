// Copyright 2025 Anthony Sanchez
#include "lamina.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
int delete_files(shared_file_data *fileobj) {
    // si el archivo binario existe, cerrarlo
    if (fileobj->file) {
        fclose(fileobj->file);
    }
    // si el archivo de reporte existe, cerrarlo
    if (fileobj->report_file) {
        fclose(fileobj->report_file);
    }
    // liberar la memoria reservada para las rutas
    free(fileobj);
    return EXIT_SUCCESS;
}
