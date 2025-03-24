// Copyright Anthony Sanchez 2024
#include <lamina.h>

#include <stdio.h>
#include <stdlib.h>

int validate_entrance(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    validate_entrance(argc, argv);
    return 0;
}

int validate_entrance(int argc, char *argv[]){
    if (argc < 2) {
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
    double time, conductivity, epsilon;
    if (fscanf(file, "%s %lf %lf %lf", bin_file, &time, &conductivity,
          &epsilon) != 4) {
        printf("Error: the file of configuration does not have the spected"
              "format\n");
        return EXIT_FAILURE;
    }

    fclose(file);

    printf("Binary file: %s\n", bin_file);
    printf("Time: %.2f\n", time);
    printf("Conductivity: %.2f\n", conductivity);
    printf("Epsilon: %.2f\n", epsilon);
    lamina_constructor(bin_file, 3);
    return EXIT_SUCCESS;
}