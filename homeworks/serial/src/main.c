// Copyright Anthony Sanchez 2024
#define _POSIX_C_SOURCE 199506L
#include <lamina.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/**
 * @brief Punto de entrada del programa.
 *
 * Mide el tiempo de ejecución de la función principal `lamina_constructor`, 
 * la cual simula el proceso térmico en una lámina.
 *
 * @param argc Número de argumentos pasados por línea de comandos.
 * @param argv Direccion del archivo txt a leer para ser procesado
 * @return 0 en caso de éxito.
 */

int main(int argc, char *argv[]) {
    struct timespec start_time;
    clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &start_time);
    lamina_constructor(argc, argv);
    struct timespec finish_time;
    clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &finish_time);
    double elapsed = (finish_time.tv_sec - start_time.tv_sec) +
      (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;
    printf("execution time: %.9lfs\n", elapsed);
    return 0;
}
