#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include "bufio.h"
#include "util.h"
#include "args.h"

#define WIDTH_FALLBACK 7

typedef struct info {
    unsigned long long lines;
    unsigned long long words;
    unsigned long long bytes;
} Info;

static Info count_info(Bufio *b) {
    Info info = {0, 0, 0};
    int byte;
    int in_word = 0; // Flag to track if we are currently inside a word

    while ((byte = bufio_next_byte(b)) != BUFIO_EOF) {
        info.bytes++; // Increment byte count for each byte read

        if (byte == '\n') {
            info.lines++; // Increment line count for each newline character
        }

        if (isspace(byte)) {
            in_word = 0; // Reset the flag when encountering whitespace
        }
        else{
            if (!in_word) {
                info.words++; // Increment word count when transitioning from whitespace to a non-whitespace character
                in_word = 1; // Set the flag indicating we are now inside a word
            }
        }
    }
    
    return info; // Return the counts of lines, words, and bytes
}

static int calc_width(Args *a) {
    int n = (a->files_count > 0) ? a->files_count : 1;
    unsigned long long bytes = 0;
    struct stat st;

    for (int i = 0; i < n; i++) {
        char *name = (a->files_count > 0) ? a->files[i] : "-";

        // stdin has no name to look up, so it goes through the descriptor
        int ok = (strcmp(name, "-") == 0) ? fstat(STDIN_FILENO, &st) == 0
                                          : stat(name, &st) == 0;
        if (!ok) {
            continue;
        }
        if (!S_ISREG(st.st_mode)) {
            return WIDTH_FALLBACK;
        }
        bytes += st.st_size;
    }

    int width = 1; // a column always holds at least one digit
    while (bytes >= 10) {
        bytes /= 10;
        width++;
    }
    return width;
}

static int print_info(Args *a) {
    Bufio b;
    Info cur;
    unsigned long long total[3] = {0, 0, 0};
    unsigned long long arr[3];  // an array for all the info for easier printing
    int len_tot = 0, k;
    int width = 0;
    int status = 0, opened_by_me, desc;
    int n = (a->files_count > 0) ? a->files_count : 1;

    // calculate the number of flags
    if (a->seen['l']) len_tot++;
    if (a->seen['w']) len_tot++;
    if (a->seen['c']) len_tot++;

    width = (a->files_count > 1) ? calc_width(a) : 0; // calculate the width for printing

    for (int i = 0; i < n; i++) {
        char *name = (a->files_count > 0) ? a->files[i] : "-";

        if (strcmp(name, "-") == 0) {desc = STDIN_FILENO; opened_by_me = 0;}  // setting descrpitor for stdin
        else {
            desc = open(name, O_RDONLY);  // setting descriptor for a regular file
            if (desc < 0) {util_warn("%s", name); status = 1; continue;}
            opened_by_me = 1;
        }

        bufio_init(&b, desc);  // initialize buffer
        cur = count_info(&b);  // count the number of lines, words and bytes

        if ((errno = bufio_error(&b))) {
            util_warn("%s", name); status = 1;
        }
        
        k = 0;
        if (a->seen['l']) {arr[k] = cur.lines; total[k] += cur.lines; k++;}
        if (a->seen['w']) {arr[k] = cur.words; total[k] += cur.words; k++;}
        if (a->seen['c']) {arr[k] = cur.bytes; total[k] += cur.bytes; k++;}

        for (int j = 0; j < len_tot; j++) { // prints info
            if (j) printf(" %*llu", width, arr[j]);
            else printf("%*llu", width, arr[j]);
        }
        if (a->files_count > 0) printf(" %s", name); // prints name of the file
        printf("\n");

        if (opened_by_me) close(desc); // close files opened by me
    }

    if (a->files_count > 1) {
        for (int j = 0; j < len_tot; j++) {  // prints total info
            if (j) printf(" %*llu", width, total[j]);
            else printf("%*llu", width, total[j]);
        }
        printf(" total\n");
    }
    return status;
}

int main(int argc, char *argv[]) {
    util_set_progname(argv[0]);

    Args args;
    const char *spec = "lwc"; // Define the accepted flags for the program

    if (args_parse(&args, argc, argv, spec)) {
        return 1; // Error parsing arguments
    }

    int cnt_flags = 0;
    for (int i = 0; spec[i] != '\0'; i++) {
        unsigned char flag = (unsigned char)spec[i];
        if (args.seen[flag]) {
            cnt_flags++; // Count the number of flags seen
        }
    }

    if (cnt_flags == 0) {
        for (int i = 0; spec[i] != '\0'; i++) args.seen[(unsigned char)spec[i]] = 1; // If no flags were seen, set all flags to seen
    }

    int status = print_info(&args);

    return status;
}
