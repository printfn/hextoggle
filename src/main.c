/* required for Visual Studio to compile */
#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bin_to_hex.h"
#include "tempfile.h"
#include "utils.h"

static const char *header = "| hextoggle output file";
enum { HEADER_LENGTH = 23 };

#if !defined(HEXTOGGLE_VERSION)
#error Make sure `HEXTOGGLE_VERSION` is set to the correct version number (e.g. `1.0.0`)
#endif
#define STR_VALUE(arg) #arg
#define STR_VALUE_2(name) STR_VALUE(name)
#define VERSION_STR STR_VALUE_2(HEXTOGGLE_VERSION)

enum StatusCode {
    StatusCodeSuccess,
    StatusCodeInvalidArgs,
    StatusCodeFailedToOpenFiles,
    StatusCodeFailedCleanup,
    StatusCodeInvalidInput,
    StatusCodeAssertionFailed,
};

typedef struct {
    int exit_with_error;   /* if non-zero, exit with that error code */
    int exit_with_success; /* if non-zero, exit successfully */
    int only_decode;
    int only_encode;
    const char *input_filename;
    const char *output_filename; /* null if we're doing a dry run */
} ParseArgsResult;

/** Validates the given command-line arguments, and prints usage description on error */
static ParseArgsResult parse_args(int argc, const char *argv[]) {
    ParseArgsResult result;
    result.exit_with_error = 0;
    result.exit_with_success = 0;
    result.only_decode = 0;
    result.only_encode = 0;
    result.input_filename = NULL;
    result.output_filename = NULL;

    bool help_arg = false;
    bool dry_run = false;
    bool valid_args = false;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            help_arg = true;
        } else if (!strcmp(argv[i], "--dry-run") || !strcmp(argv[i], "-n")) {
            dry_run = true;
        } else if (!strcmp(argv[i], "--decode") || !strcmp(argv[i], "-d")) {
            result.only_decode = true;
        } else if (!strcmp(argv[i], "--encode") || !strcmp(argv[i], "-e")) {
            result.only_encode = true;
        } else {
            if (!result.input_filename) {
                result.input_filename = argv[i];
                valid_args = true;
            } else if (!result.output_filename) {
                result.output_filename = argv[i];
            } else {
                /* too many args */
                valid_args = false;
            }
        }
    }

    if (!dry_run && !result.output_filename) {
        result.output_filename = result.input_filename;
    } else if (dry_run) {
        result.output_filename = NULL;
    }

    if (result.only_decode && result.only_encode) {
        valid_args = false;
    }

    if (valid_args && !help_arg) {
        return result;
    }

    fprintf(stderr,
        "hextoggle v" VERSION_STR "\n"
        "\n"
        "Usage: hextoggle [file]            # toggle file in-place\n"
        "       hextoggle [input] [output]  # read 'input', write to 'output'\n"
        "       hextoggle -                 # read from stdin/write to stdout\n"
        "\n"
        "Flags:\n"
        "       -n        --dry-run         # discard results\n"
        "       -d        --decode          # force decode (i.e. hex -> binary)\n"
        "       -e        --encode          # force encode (i.e. binary -> hex)\n"
        "       -h        --help            # show this usage information\n"
        "\n");
    fprintf(stderr, "Return codes:\n");
    fprintf(stderr, "  %i   success\n", EXIT_SUCCESS);
    fprintf(stderr, "  %i   invalid arguments\n", StatusCodeInvalidArgs);
    fprintf(stderr, "  %i   failed to open input files\n", StatusCodeFailedToOpenFiles);
    fprintf(stderr, "  %i   failed to clean up files\n", StatusCodeFailedCleanup);
    fprintf(stderr, "  %i   invalid input\n", StatusCodeInvalidInput);
    fprintf(stderr, "  %i   internal assertion failed\n", StatusCodeAssertionFailed);

    if (help_arg) {
        result.exit_with_success = 1;
    } else {
        result.exit_with_error = StatusCodeInvalidArgs;
    }
    return result;
}

static int cleanup_files(FILE *input, FILE *temp_output,
                  const char *temp_output_filename,
                  const char *real_output_filename) {
    if (temp_output && temp_output != stdout) {
        fclose(temp_output);
    }
    if (input != stdin) {
        fclose(input);
    }
    if (!real_output_filename)
        return 0;
    if (!temp_output_filename[0]) {
        /* we wrote directly to the target file, so no need to rename things */
        return 0;
    }
#ifdef _MSC_VER
    if (-1 == remove(real_output_filename)) {
        if (errno != ENOENT) {
            // target file does exist but we cannot delete it
            fprintf(stderr, "Unable to remove file '%s': %s\n",
                temp_output_filename, strerror(errno));
            return 1;
        }
    }
#endif
    if (-1 == rename(temp_output_filename, real_output_filename)) {
        fprintf(stderr, "Unable to rename file '%s' to '%s': %s\n",
            temp_output_filename, real_output_filename, strerror(errno));
        return 1;
    }
    return 0;
}

typedef struct {
    int inside_comment; /* potentially nested comments */
    bool skip_line;
    char prev_byte;
} FromHexData;

static FromHexData init_from_hex_data() {
    FromHexData result;
    result.prev_byte = 0;
    result.skip_line = false;
    result.inside_comment = 0;
    return result;
}

static int hex_to_chars(FromHexData *data, char c, FILE *output_stream) {
    if (data->prev_byte) {
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
            int first_char = hex_char_to_int(data->prev_byte);
            if (first_char < 0) {
                return 1;
            }
            char output = (char)(first_char << 4);
            int second_char = hex_char_to_int(c);
            if (second_char < 0) {
                return 1;
            }
            output += second_char;
            if (output_stream) {
                fputc(output, output_stream);
            }
            data->prev_byte = 0;
            return 0;
        } else {
            return 1;
        }
    }

    /* It's important to process skipped lines BEFORE block comments because
       otherwise '[' and ']' characters on the right-hand side can unintentionally
       break the file. */
    if (data->skip_line && c != '\n') {
        return 0;
    }
    if (c == '\n') {
        data->skip_line = false;
        return 0;
    }
    if (c == '|') {
        data->skip_line = true;
        return 0;
    }

    if (c == '[') {
        ++data->inside_comment;
        return 0;
    }
    if (data->inside_comment && c == ']') {
        --data->inside_comment;
        return 0;
    }
    if (data->inside_comment) {
        return 0;
    }

    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
        data->prev_byte = c;
        return 0;
    }
    if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
        return 0;
    }
    return 1;
}

/* Return values: 0 for success, 1 for retry as to_hex, 2 for error */
static int try_from_hex(FILE *input_file, FILE *output_file,
                 char *from_hex_read_buffer, int *from_hex_read_buffer_length,
                 bool check_header) {
    int ci;
    unsigned long long i = 0;
    unsigned long long line_no = 1;
    unsigned long long col_no = 0;
    FromHexData data = init_from_hex_data();
    while ((ci = fgetc(input_file)) != EOF) {
        char c = (char)ci;
        if (c == '\n') {
            ++line_no;
            col_no = 0;
        } else {
            ++col_no;
        }
        if (check_header && i < HEADER_LENGTH) {
            from_hex_read_buffer[*from_hex_read_buffer_length] = c;
            ++*from_hex_read_buffer_length;
            if (header[i] != c) {
                return 1;
            }
        }
        if (hex_to_chars(&data, c, output_file)) {
            fprintf(stderr, "Invalid format at character %llu, line %llu, col %llu, aborting\n", i, line_no, col_no);
            return 2;
        }
        ++i;
    }
    if (check_header && i < HEADER_LENGTH) {
        /* header was incomplete (or file was empty) */
        return 1;
    }
    
    return 0;
}

static int try_to_hex(FILE *input_file, FILE *output_file,
               const char *from_hex_read_buffer, int from_hex_read_buffer_length) {
    if (output_file) {
        fputs(header, output_file);
        fputc('\n', output_file);
    }
    
    // BLOCK_BATCH describes the number of blocks (sets of 16 bytes)
    // to convert at once. Each block produces 81 bytes of output.
    enum { BLOCK_BATCH = 64 };
    uint64_t addr = 0;
    char output[81 * BLOCK_BATCH];
    char input[16 * BLOCK_BATCH];
    
    /* read in chars already read in by try_from_hex */
    memcpy(input, from_hex_read_buffer, from_hex_read_buffer_length);
    
    while (true) {
        size_t i = from_hex_read_buffer_length
            + fread(input + from_hex_read_buffer_length,
                    1,
                    16 * BLOCK_BATCH - from_hex_read_buffer_length,
                    input_file);
        from_hex_read_buffer_length = 0;
        uint64_t output_data_len = bin_data_to_hex(input, i, addr, output);
        addr += i;
        if (output_file) {
            fwrite(output, output_data_len, 1, output_file);
        }
        if (i < 16 * BLOCK_BATCH) {
            break;
        }
    }
    return 0;
}

static int open_files(FILE **input_file, FILE **output_file,
               const char *input_filename, const char *output_filename,
               char *output_filename_buffer) {
    if (!strcmp(input_filename, "-")) {
        *input_file = stdin;
    } else {
        *input_file = fopen(input_filename, "rb");
        if (!*input_file) {
            fprintf(stderr, "Unable to open file '%s' for reading: %s\n", input_filename, strerror(errno));
            return StatusCodeFailedToOpenFiles;
        }
    }

    *output_file = NULL;
    if (output_filename) {
        if (!strcmp(output_filename, "-")) {
            *output_file = stdout;
        } else if (!strcmp(output_filename, input_filename)) {
            /* if the filenames are the same we need a temp file */
            *output_file = open_temporary_file(output_filename_buffer);
        } else {
            /* otherwise open the file directly */
            *output_file = fopen(output_filename, "wb");
            if (!*output_file) {
                fprintf(stderr, "Unable to open file '%s' for writing: %s\n", output_filename, strerror(errno));
                if (*input_file) {
                    fclose(*input_file);
                }
                return StatusCodeFailedToOpenFiles;
            }
        }
    }

    return 0;
}

int main(int argc, const char *argv[]) {
    ParseArgsResult parse_args_result = parse_args(argc, argv);
    if (parse_args_result.exit_with_error) {
        return parse_args_result.exit_with_error;
    } else if (parse_args_result.exit_with_success) {
        return EXIT_SUCCESS;
    }

    const char *input_filename = parse_args_result.input_filename;
    const char *output_filename = parse_args_result.output_filename;

    FILE *input_file, *output_file;
    char output_filename_buffer[TEMP_FILENAME_SIZE];
    output_filename_buffer[0] = '\0';
    if (open_files(&input_file, &output_file,
                   input_filename, output_filename, output_filename_buffer)) {
        return StatusCodeFailedToOpenFiles;
    }
    
    /* store read characters in case we need to retry as to_hex. */
    int from_hex_read_buffer_length = 0;
    char from_hex_read_buffer[HEADER_LENGTH] = {0};

    if (parse_args_result.only_encode) {
        goto try_encode;
    }
    
    int res = try_from_hex(
        input_file, output_file,
        from_hex_read_buffer, &from_hex_read_buffer_length,
        !parse_args_result.only_decode);
    switch (res) {
        case 0: /* success */
            if (cleanup_files(input_file, output_file, output_filename_buffer, output_filename)) {
                return StatusCodeFailedCleanup;
            }
            return EXIT_SUCCESS;
        case 2: /* failure */
            fclose(input_file);
            if (output_file) {
                fclose(output_file);
                if (output_filename_buffer[0]) {
                    remove(output_filename_buffer);
                }
            }
            return StatusCodeInvalidInput;
        case 1: /* retry */
        try_encode:
            res = try_to_hex(input_file, output_file, from_hex_read_buffer, from_hex_read_buffer_length);
            if (res == 0) {
                if (cleanup_files(input_file, output_file, output_filename_buffer, output_filename)) {
                    return StatusCodeFailedCleanup;
                }
                return EXIT_SUCCESS;
            }
            fclose(input_file);
            if (output_file) {
                fclose(output_file);
                if (output_filename_buffer[0]) {
                    remove(output_filename_buffer);
                }
            }
            return StatusCodeInvalidInput;
    }
    return StatusCodeAssertionFailed;
}
