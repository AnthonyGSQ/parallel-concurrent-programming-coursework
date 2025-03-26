// Copyright 2025 Anthony Sanchez
#ifndef LAMINA_H
#define LAMINA_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint64_t rows;
    uint64_t columns;
    uint64_t k;
    uint64_t time;
    double conductivity;
    double height;
    double epsilon;
    double ** temperatures;
    double ** next_temperatures;
    FILE *file;
} Lamina;
// Funcion encargada del recibimiento de los parametros, abrir los archivos
// crear el objeto lamina y llamar a las funciones validate_entrance(),
// reading_parameters() y finalmente a create_lamina().
// En caso de recibir parametros invalidos, no encontrar el archivo indicado
// por el usuario, no poder abrir el archivo encontrado o no poder crear
// la lamina, retorna EXIT_FAILURE
int lamina_constructor(int argc, char *argv[]);
// Funcion encargada de verificar que los parametros recibidos del txt
// sean validos, ej: Que height no sea 0
int reading_parameters(Lamina *lamina, char* filename);
// Funcion encargada de reservar memoria para las matrices de la interfaz
// temperatures y next_temperatures. Lee el archivo binario, tomando los
// primeros 8 bytes como el total de filas, los segundos 8 bytes como numero
// de columnas y el resto como los valores de la lamina.
// En caso de no encontrar el archivo binario, no poder reservar memoria
// para las matrices o recibir un numero invalido de filas o columnas
// retorna EXIT_FAILURE
int create_lamina(Lamina *lamina);
// Funcion encargada de actualizar la lamina del estado k al estado k+1
// va aumentando la temperatura, realizando llamados a update_cell() solo
// si no se esta trabajando con un borde, si la celda actual es un borde
// la ignora.
void update_lamina(Lamina * lamina);
// Funcion encargada de actualizar la celda de la posicion recibida,
// utilizando la formula de difusion de calor y guardandola en la
// misma posicion pero en lamina->next_temperatures.
void update_cell(Lamina * lamina, uint64_t row, uint64_t column);
void finish_simulation(Lamina * lamina);
void print_lamina(Lamina * lamina);
void delete_lamina();

#endif  // HOMEWORKS_SERIAL_SRC_LAMINA_H_
