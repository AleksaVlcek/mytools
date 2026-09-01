#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "bufio.h"
#include "util.h"

int main(int argc, char *argv[]) {
    util_set_progname(argv[0]);

    Bufio b; int er;
    const char *source = "-"; // Default to standard input
    bufio_init(&b, STDIN_FILENO); // Initialize buffer for reading from standard input

    unsigned long long cnt = 0;
    int byte;

    while ((byte = bufio_next_byte(&b)) != BUFIO_EOF) {  // Read bytes until end of file
        if (byte == '\n') {
            cnt++;          // Increment count for each newline character
        }
    }
    if ((er = bufio_error(&b))) {
        errno = er;
        util_warn("%s", source); // Print error message if an error occurred during reading
        return 1;
    }

    printf("%llu\n", cnt); // Print the count of newline characters

    return 0;
}
