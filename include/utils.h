#ifndef UTILS_H
#define UTILS_H

char *read_file(const char *path);
int has_lp_extension(const char *path);
void print_error(const char *stage, int line, int column, const char *message);

#endif