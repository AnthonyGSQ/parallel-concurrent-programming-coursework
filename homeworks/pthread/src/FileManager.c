// Copyright 2025 Anthony Sanchez
#include "lamina.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
    // si no se encuentra la carpeta tsv, crea el archivo tsv en la misma
    // carpeta donde se encuentra el binario leido
    fileobj->report_file = fopen(temp, "w+");
    if (!fileobj->report_file) {
        printf("Error: could not open tsv file\n");
        printf("Creating new tsv file\n");
        fileobj->report_file = fopen(alternative_route, "w+");
    }
    snprintf(text, sizeof(temp), "%s\t%.lf \t%g\t%g\t%g\t%g\t", temp,
    lamina->time, lamina->conductivity, lamina->height,
    lamina->epsilon, lamina->k);
    format_time(lamina->k, text, sizeof(text));
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
