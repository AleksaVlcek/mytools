#ifndef ARGS_H
#define ARGS_H

#include <limits.h>
#define ARGS_TABLE_SIZE (UCHAR_MAX + 1)

typedef struct args {
    unsigned char seen[ARGS_TABLE_SIZE];
    const char *value[ARGS_TABLE_SIZE];
    char **files;
    int files_count;
} Args;

int args_parse(Args *args, int argc, char *argv[], const char *spec); // parses argv into *args; spec lists the flag letters, a ':' after one means it takes a value; returns 0 on success

void args_usage(void); // prints one usage line to stderr, naming the tool from util_get_progname()

#endif // ARGS_H
