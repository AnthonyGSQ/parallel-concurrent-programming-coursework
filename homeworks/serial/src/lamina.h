#ifdef LAMINA_H
#define LAMINA_H

#inlude <stdio.h>
#include <stdlib.h>

typedef struct {
    int rows;
    int columns;
    float ** temperatures;
} Lamina;

