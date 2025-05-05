// Copyright 2025 Anthony Sanchez
#include "lamina.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
int delete_files(shared_file_data *fileobj) {
    if (fileobj->file) {
        fclose(fileobj->file);
    }
    if (fileobj->report_file) {
        fclose(fileobj->report_file);
    }
    free(fileobj);
    return EXIT_SUCCESS;
}
