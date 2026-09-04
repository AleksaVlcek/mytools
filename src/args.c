#include "args.h"
#include "util.h"
#include <string.h>
#include <stdio.h>

// TODO: args_usage cannot know the spec, so the flag list is still wc's
// Give it a parameter once head and tail need their own usage line
void args_usage(void) {
    fprintf(stderr, "usage: %s [-lwc] [file...]\n", util_get_progname());
}

// Looks up flag in spec and returns its position there, or NULL if it is not
// an accepted flag. A ':' in spec marks the flag before it as taking a value,
// so ':' itself is never a flag
static const char *spec_find(const char *spec, unsigned char flag) {
    if (flag == '\0' || flag == ':') {
        return NULL;
    }
    return strchr(spec, flag);
}

int args_parse(Args *args, int argc, char *argv[], const char *spec) {
    memset(args, 0, sizeof *args); // caller need not initialise the struct

    int i = 1; // skip the program name in argv[0]

    while (i < argc) {
        char *t = argv[i];

        if (strcmp(t, "--") == 0) { // end of options, operands start after it
            i++;
            break;
        }
        if (strcmp(t, "-") == 0 || t[0] != '-') { // operand: a file name, "-" being standard input
            break;
        }

        for (int j = 1; t[j] != '\0'; j++) { // a bundle: -lw is -l -w
            unsigned char flag = (unsigned char)t[j];
            const char *pos = spec_find(spec, flag);

            if (pos == NULL) {
                util_warnx("invalid option -- '%c'", flag);
                args_usage();
                return 1; // a wrong call is never applied by halves
            }
            args->seen[flag] = 1;

            if (pos[1] == ':') { // takes a value, which ends the bundle
                if (t[j + 1] != '\0') {
                    args->value[flag] = &t[j + 1]; // attached, as in -n20
                }
                else if (i + 1 < argc) {
                    args->value[flag] = argv[++i]; // separate, as in -n 20
                }
                else {
                    util_warnx("option requires an argument -- '%c'", flag);
                    args_usage();
                    return 1;
                }
                break;
            }
        }
        i++;
    }

    args->files = &argv[i]; // points into argv; argv[argc] exists and is NULL
    args->files_count = argc - i;

    return 0;
}
