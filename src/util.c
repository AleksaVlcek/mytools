#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static const char *progname = "?";

void util_set_progname(const char *name) {
    if (name != NULL && name[0] != '\0') {
        progname = name;    // full path to the program
    }
}

const char *util_get_progname(void) {
    return progname;
}

void util_warn(const char *format, ...) {
    int saved_errno = errno;
    fprintf(stderr, "%s: ", progname);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, ": %s\n", strerror(saved_errno));
}

void util_warnx(const char *format, ...) {
    fprintf(stderr, "%s: ", progname);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}
