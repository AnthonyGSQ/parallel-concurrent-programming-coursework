// Copyright Anthony Sanchez 2024
#include <lamina.h>

#include <stdio.h>
#include <stdlib.h>

int validate_entrance(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    validate_entrance(argc, argv);
    return 0;
}
// funcion encargada de verificar que el archivo txt a leer posea los 5
// parametros esperados, en caso de que argv no tenga los argumentos esperados
// retorna failure, si no encuentra el archivo txt, retorna failure
// si el archivo txt no posee los 5 parametros esperados, retorna
int validate_entrance(int argc, char *argv[]){
    if (argc < 3) {
        printf("Invalid entrance: you need to specify a .txt and the amounth of"
            "threads you want to use\n");
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Error: invalid file\n");
        return 1;
    }
    char bin_file[256];
    char temp[256];
    char line[256];
    double time, conductivity, height, epsilon;
        printf("%s", line);

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%s %lf %lf %lf %lf", temp, &time, &conductivity,
            &height, &epsilon) != 5) {
        printf("Error: the file of configuration does not have the spected "
            "format\n");
        return EXIT_FAILURE;
        }
        snprintf(bin_file, sizeof(bin_file), "../jobs/%s", temp);
        printf("Binary file: %s\n", bin_file);
        printf("Time: %.2f\n", time);
        printf("Conductivity: %.2f\n", conductivity);
        printf("Epsilon: %.2f\n", epsilon);
        lamina_constructor(bin_file, 3);
    }
    fclose(file);
    return EXIT_SUCCESS;
}