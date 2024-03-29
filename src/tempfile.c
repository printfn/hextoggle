#define _POSIX_C_SOURCE 200809L

#include "tempfile.h"

#include "utils.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *handle_errors(FILE *file, const char *filename);

#ifdef _MSC_VER

FILE *open_temporary_file(char *filename) {
    errno_t err;
    FILE *file;

    err = tmpnam_s(filename, L_tmpnam_s);
    if (err) {
        fprintf(stderr,
            "Error: Unable to get temp file name: %s\n",
            strerror(err));
        return NULL;
    }
    file = fopen(filename, "wb");
    return handle_errors(file, filename);
}

#else

#include <unistd.h>
FILE *open_temporary_file(char *filename) {
    int fd;
    BOOL success;
    FILE *file;

    strcpy(filename, ".temp_hextoggle_XXXXXXXX");
    fd = mkstemp(filename);
    success = fd != -1;
    if (!success) {
        fprintf(stderr,
            "Error: Unable to get temp file name: %s\n",
            strerror(errno));
        return NULL;
    }
    file = fdopen(fd, "wb");
    return handle_errors(file, filename);
}

#endif /* _MSC_VER */

static FILE *handle_errors(FILE *file, const char *filename) {
    if (file) {
        return file;
    }
    fprintf(stderr,
        "Error: Unable to open temporary file `%s` for writing: %s\n",
        filename, strerror(errno));
    remove(filename);
    return NULL;
}
