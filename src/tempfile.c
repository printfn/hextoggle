#include "tempfile.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static FILE *handle_errors(FILE *file, const char *filename);

#ifdef _MSC_VER

FILE *open_temporary_file(char *filename) {
    bool success = tmpnam_s(filename, L_tmpnam_s) == NULL;
    if (!success) {
        fprintf(stderr, "Unable to get temp file name: %s\n", strerror(errno));
        return NULL;
    }
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Unable to open temporary file '%s' for writing: %s\n", filename, strerror(errno));
        remove(filename);
        return NULL;
    }
    return handle_errors(file, filename);
}

#else

#include <unistd.h>
FILE *open_temporary_file(char *filename) {
    strcpy(filename, ".temp_hextoggle_XXXXXXXX");
    int fd = mkstemp(filename);
    bool success = fd != -1;
    if (!success) {
        fprintf(stderr, "Unable to get temp file name: %s\n", strerror(errno));
        return NULL;
    }
    FILE *file = fdopen(fd, "wb");
    return handle_errors(file, filename);
}

#endif // _MSC_VER

static FILE *handle_errors(FILE *file, const char *filename) {
    if (file) {
        return file;
    }
    fprintf(stderr, "Unable to open temporary file `%s` for writing: %s\n", filename, strerror(errno));
    remove(filename);
    return NULL;
}
