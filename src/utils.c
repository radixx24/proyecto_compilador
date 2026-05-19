#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

char *read_file(const char *path) {
    FILE *file;
    long size;
    char *buffer;
    size_t read_size;

    file = fopen(path, "rb");

    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    buffer = (char *)malloc((size_t)size + 1);

    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    read_size = fread(buffer, 1, (size_t)size, file);
    buffer[read_size] = '\0';

    fclose(file);

    return buffer;
}

int has_lp_extension(const char *path) {
    size_t length;

    length = strlen(path);

    if (length < 3) {
        return 0;
    }

    if (path[length - 3] == '.' && path[length - 2] == 'l' && path[length - 1] == 'p') {
        return 1;
    }

    return 0;
}

void print_error(const char *stage, int line, int column, const char *message) {
    printf("[%s] Error en linea %d, columna %d: %s\n", stage, line, column, message);
}