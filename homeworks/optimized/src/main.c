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
    // Declaración de una estructura para almacenar el tiempo de inicio
    struct timespec start_time;
    // Se obtiene el tiempo actual antes de iniciar el procesamiento principal
    clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &start_time);
    // Llama a la función principal que simula el proceso térmico en la lámina,
    // pasando los argumentos de línea de comandos
    lamina_constructor(argc, argv);
    // Declaración de una estructura para almacenar el tiempo de finalización
    struct timespec finish_time;
    // Se obtiene el tiempo actual después de finalizar el procesamiento
    // principal
    clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &finish_time);
    // Calcula el tiempo transcurrido en segundos como número de punto flotante
    double elapsed = (finish_time.tv_sec - start_time.tv_sec) +
      (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;
    // Imprime el tiempo de ejecución en la consola con 9 decimales de precisión
    printf("execution time: %.9lfs\n", elapsed);
    // Retorna 0 para indicar que el programa terminó correctamente
    return 0;
}
