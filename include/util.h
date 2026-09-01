#ifndef UTIL_H
#define UTIL_H

#ifdef __GNUC__
#define UTIL_FMT_ARG(a, b)  __attribute__((format(printf, a, b)))
#else
#define UTIL_FMT_ARG(a, b)
#endif

void util_set_progname(const char *name);  // sets the name of the program

const char *util_get_progname(void);  // gets the name of the program

void util_warn(const char *format, ...) UTIL_FMT_ARG(1, 2);  // prints the error message associated with errno

void util_warnx(const char *format, ...) UTIL_FMT_ARG(1, 2);  // prints a message without errno

#endif // UTIL_H
