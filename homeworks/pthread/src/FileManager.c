// Copyright 2025 Anthony Sanchez
#include "lamina.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int report_file(Lamina *lamina, file_struct *fileobj) {
    // variable para guardar el texto que se escribira en el archivo tsv
    char text[260] = "";
    char temp[260] = "";
    // variable con el nombre del archivo tsv donde se escribira el reporte
    char file_name[260] = "";
    // variable para guardar la ruta alternativa del archivo tsv
    char alternative_route[260] = "";
    // conseguimos el nombre del archivo binario
    snprintf(file_name, sizeof(file_name), "%s", fileobj->binary_file_name);
    char *lastSLash = strrchr(file_name, '/') + 1;
    char *dot = strrchr(file_name, '.');
    *dot = '\0';
    // creamos una ruta alternativa en caso de que no se encuentre la carpeta
    snprintf(alternative_route, sizeof(alternative_route), "%s%s%s",
    fileobj->base_route, lastSLash, ".tsv");
    // creamos la ruta del archivo tsv donde se guardara el reporte de la lamina
    snprintf(temp, sizeof(temp), "%s%s%s%s", fileobj->base_route,
    "tsv/", lastSLash, ".tsv");
    // si no se encuentra la carpeta tsv, crea el archivo tsv en la misma
    // carpeta donde se encuentra el binario leido
    fileobj->report_file = fopen(temp, "w+");
    if (!fileobj->report_file) {
        printf("Error: could not open tsv file\n");
        printf("Creating new tsv file\n");
        fileobj->report_file = fopen(alternative_route, "w+");
    }
    // conseguimos la cantidad de tiempo transcurrida en la simulacion
    double time = lamina->time / lamina->k;
    // armamos el texto que se escribira en el archivo tsv
    snprintf(text, sizeof(fileobj->binary_file_name),
    "%s.bin\t%.lf \t%g\t%g\t%g\t%g\t", lastSLash,
    time, lamina->conductivity, lamina->height,
    lamina->epsilon, lamina->k);
    // formateamos el tiempo a una escritura legible para una persona
    format_time(lamina->time, text, sizeof(text));
    fprintf(fileobj->report_file, "%s", text);
    return EXIT_SUCCESS;
}
int delete_files(file_struct *fileobj) {
    // si existe el archivo binario, lo cerramos
    if (fileobj->file) {
        fclose(fileobj->file);
    }
    // si existe el archivo tsv, lo cerramos
    if (fileobj->report_file) {
        fclose(fileobj->report_file);
    }
    // liberamos la memoria reservada para el objeto encargado de manejar los
    free(fileobj);
    return EXIT_SUCCESS;
}
