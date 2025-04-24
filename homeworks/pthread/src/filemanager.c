// Copyright 2025 Anthony Sanchez
#include "lamina.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
 *
 * @return EXIT_SUCCESS Si el reporte fue exitoso, EXIT_FAILURE si no
 *
 * @note Esta función intenta abrir un archivo en la ruta proporcionada por la
 * estructura `Lamina`. Si no se puede abrir, intentará crear un archivo en una
 * ruta alternativa.
 */
int report_file(Lamina *lamina, file_struct *fileobj) {
    char text[260] = "";
    char temp[260] = "";
    char file_name[260] = "";
    char alternative_route[260] = "";
    snprintf(file_name, sizeof(file_name), "%s", fileobj->binary_file_name);
    char *lastSLash = strrchr(file_name, '/') + 1;
    char *dot = strrchr(file_name, '.');
    *dot = '\0';
    snprintf(alternative_route, sizeof(alternative_route), "%s%s%s",
    fileobj->base_route, lastSLash, ".tsv");
    snprintf(temp, sizeof(temp), "%s%s%s%s", fileobj->base_route,
    "tsv/", lastSLash, ".tsv");
    printf("HOLA: %s\n", lastSLash);
    // si no se encuentra la carpeta tsv, crea el archivo tsv en la misma
    // carpeta donde se encuentra el binario leido
    fileobj->report_file = fopen(temp, "w+");
    if (!fileobj->report_file) {
        printf("Error: could not open tsv file\n");
        printf("Creating new tsv file\n");
        fileobj->report_file = fopen(alternative_route, "w+");
    }
    double time = lamina->time / lamina->k;
    snprintf(text, sizeof(fileobj->binary_file_name),
    "%s.bin\t%.lf \t%g\t%g\t%g\t%g\t", lastSLash,
    time, lamina->conductivity, lamina->height,
    lamina->epsilon, lamina->k);
    format_time(lamina->time, text, sizeof(text));
    fprintf(fileobj->report_file, "%s", text);
    return EXIT_SUCCESS;
}

int delete_files(file_struct *fileobj) {
    if (fileobj->file) {
        fclose(fileobj->file);
    }
    if (fileobj->report_file) {
        fclose(fileobj->report_file);
    }
    free(fileobj);
    return EXIT_SUCCESS;
}
