#include <stdio.h>
#include <unistd.h>
#include "bufio.h"

int main(void) {
    Bufio b;
    bufio_init(&b, STDIN_FILENO); // Initialize buffer for reading from standard input

    unsigned long long cnt = 0;
    int byte;

    while ((byte = bufio_next_byte(&b)) != BUFIO_EOF) {  // Read bytes until end of file
        if (byte == '\n') {
            cnt++;          // Increment count for each newline character
        }
    }

    printf("%llu\n", cnt); // Print the count of newline characters

    return 0;
}