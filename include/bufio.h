#ifndef BUFIO_H
#define BUFIO_H

#include <stddef.h>
#define BUFIO_SIZE (64 * 1024)
#define BUFIO_EOF (-1)

typedef struct bufio {
    int fd;
    unsigned char buf[BUFIO_SIZE];
    size_t len;
    size_t pos;
    int eof;
    int err;
} Bufio;

void bufio_init(Bufio *b, int fd); // initializes the buffer for reading from file descriptor fd

int bufio_next_byte(Bufio *b); // returns byte 0-255, or BUFIO_EOF on end of file

int bufio_error(Bufio *b); // returns errno value if an error occurred during reading, or 0 if no error occurred

#endif // BUFIO_H
