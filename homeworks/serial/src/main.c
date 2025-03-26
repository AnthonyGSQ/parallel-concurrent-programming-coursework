// Copyright Anthony Sanchez 2024
#include <lamina.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    lamina_constructor(argc, argv);
    return 0;
}
// funcion encargada de verificar que el archivo txt a leer posea los 5
// parametros esperados, en caso de que argv no tenga los argumentos esperados
// retorna failure, si no encuentra el archivo txt, retorna failure
// si el archivo txt no posee los 5 parametros esperados, retorna