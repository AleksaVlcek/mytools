#include "bufio.h"
#include <unistd.h>
#include <errno.h>

void bufio_init(Bufio *b, int fd) {
    b->fd = fd;
    b->len = 0;
    b->pos = 0;
    b->eof = 0;
    b->err = 0;
}

static void bufio_fill(Bufio *b) {
    ssize_t n = read(b->fd, b->buf, BUFIO_SIZE);
    while (n < 0 && errno == EINTR) {            // Retry on interrupt
        n = read(b->fd, b->buf, BUFIO_SIZE);
    }
    if (n > 0) {              // Successfully read some bytes
        b->len = n;
        b->pos = 0;
    }
    else if (n == 0) {         //EOF
        b->eof = 1;
        b->len = 0;
        b->pos = 0;
    }
    else {                    // Error
        b->err = errno;
        b->len = 0;
        b->pos = 0;
    }
}

int bufio_next_byte(Bufio *b) {
    while (b->pos == b->len) {
        if (b->eof || b->err) {
            return BUFIO_EOF; // Return EOF on end of file and on error
        }
        bufio_fill(b);
    }
    return b->buf[b->pos++];
}

int bufio_error(Bufio *b) {
    return b->err;
}
