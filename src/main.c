/*
    Thanks for reading this code!
    Feel free to contribute to this project if you'd like :)
*/

/* required for Visual Studio to compile */
#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include "bin_to_hex.h"
#include "tempfile.h"
#include "utils.h"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *header = "| hextoggle output file";
enum { HEADER_LENGTH = 23 };

#if !defined(HEXTOGGLE_VERSION)
/* be careful not to indent the line after the backslash */
#error Make sure `HEXTOGGLE_VERSION` is set to the \
correct version number (e.g. `1.0.0`)
#endif
#define STR_VALUE(arg) #arg
#define STR_VALUE_2(name) STR_VALUE(name)
#define VERSION_STR STR_VALUE_2(HEXTOGGLE_VERSION)

enum StatusCode {
    StatusCodeInvalidArgs = 1,
    StatusCodeFailedToOpenFiles,
    StatusCodeFailedCleanup,
    StatusCodeInvalidInput,
    StatusCodeAssertionFailed
};

typedef enum Conversion {
    ConversionAutoDetect,
    ConversionOnlyDecode,
    ConversionOnlyEncode
} Conversion;

typedef struct {
    int exit_with_error;   /* if non-zero, exit with that error code */
    int exit_with_success; /* if non-zero, exit successfully */
    Conversion conversion;
    const char *input_filename;
    const char *output_filename; /* null if we're doing a dry run */
} ParseArgsResult;

/* main usage string, but doesn't include return codes */
static const char *USAGE_STRING = 
"hextoggle v" VERSION_STR "\n"
"\n"
"Usage: hextoggle [file]            # toggle file in-place\n"
"       hextoggle [input] [output]  # read `input`, write to `output`\n"
"       hextoggle -                 # read from stdin/write to stdout\n"
"\n"
"Options:\n"
"       -n  --dry-run  # discard results\n"
"       -d  --decode   # force decode (i.e. hex -> binary)\n"
"       -e  --encode   # force encode (i.e. binary -> hex)\n"
"       -h  --help     # show this usage information\n"
"\n";

/** Validate the given command-line arguments,
and print usage description on error */
static ParseArgsResult parse_args(int argc, const char *argv[]) {
    ParseArgsResult result;
    BOOL help_arg, dry_run, valid_args, raw_args;
    int i;

    result.exit_with_error = 0;
    result.exit_with_success = 0;
    result.conversion = ConversionAutoDetect;
    result.input_filename = NULL;
    result.output_filename = NULL;

    help_arg = FALSE;
    dry_run = FALSE;
    valid_args = FALSE;
    raw_args = FALSE;
    for (i = 1; i < argc; ++i) {
        if (raw_args || strncmp("-", argv[i], 1)) {
            if (!result.input_filename) {
                result.input_filename = argv[i];
                valid_args = TRUE;
            } else if (!result.output_filename) {
                result.output_filename = argv[i];
            } else {
                /* too many args */
                valid_args = FALSE;
            }
        } else if (!strcmp(argv[i], "--help")
                || !strcmp(argv[i], "-h")) {
            help_arg = TRUE;
        } else if (!strcmp(argv[i], "--dry-run")
                || !strcmp(argv[i], "-n")) {
            dry_run = TRUE;
        } else if (!strcmp(argv[i], "--decode")
                || !strcmp(argv[i], "-d")) {
            result.conversion = ConversionOnlyDecode;
        } else if (!strcmp(argv[i], "--encode")
                || !strcmp(argv[i], "-e")) {
            result.conversion = ConversionOnlyEncode;
        } else if (!strcmp(argv[i], "--")) {
            raw_args = TRUE;
        } else {
            /* unknown argument */
            valid_args = FALSE;
        }
    }

    if (!dry_run && !result.output_filename) {
        result.output_filename = result.input_filename;
    } else if (dry_run) {
        result.output_filename = NULL;
    }

    if (valid_args && !help_arg) {
        return result;
    }

    fprintf(stderr, "%s", USAGE_STRING);
    fprintf(stderr, "Return codes:\n");
    fprintf(stderr, "  %i   success\n", EXIT_SUCCESS);
    fprintf(stderr, "  %i   invalid arguments\n",
        StatusCodeInvalidArgs);
    fprintf(stderr, "  %i   failed to open input files\n",
        StatusCodeFailedToOpenFiles);
    fprintf(stderr, "  %i   failed to clean up files\n",
        StatusCodeFailedCleanup);
    fprintf(stderr, "  %i   invalid input\n",
        StatusCodeInvalidInput);
    fprintf(stderr, "  %i   internal assertion failed\n",
        StatusCodeAssertionFailed);

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
        /* we wrote directly to the target file,
            so no need to rename things */
        return 0;
    }
#ifdef _MSC_VER
    if (-1 == remove(real_output_filename)) {
        if (errno != ENOENT) {
            /* target file does exist but we cannot delete it */
            fprintf(stderr, "Unable to remove file `%s`: %s\n",
                temp_output_filename, strerror(errno));
            return 1;
        }
    }
#endif
    if (-1 == rename(temp_output_filename, real_output_filename)) {
        fprintf(stderr, "Unable to rename file `%s` to `%s`: %s\n",
            temp_output_filename,
            real_output_filename,
            strerror(errno));
        return 1;
    }
    return 0;
}

typedef struct {
    int inside_comment; /* potentially nested comments */
    BOOL skip_line;
    char prev_byte;
} FromHexData;

static FromHexData init_from_hex_data() {
    FromHexData result;
    result.prev_byte = 0;
    result.skip_line = FALSE;
    result.inside_comment = 0;
    return result;
}

static int hex_to_chars(
        FromHexData *data, char c, FILE *output_stream) {
    int first_char, second_char;
    char output;

    if (data->prev_byte) {
        if ((c >= '0' && c <= '9')
                || (c >= 'a' && c <= 'f')
                || (c >= 'A' && c <= 'F')) {
            first_char = hex_char_to_int(data->prev_byte);
            if (first_char < 0) {
                return 1;
            }
            output = (char)(first_char << 4);
            second_char = hex_char_to_int(c);
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

    /* It's important to process skipped lines BEFORE block comments
        because otherwise '[' and ']' characters on the right-hand side
        can unintentionally break the file. */
    if (data->skip_line && c != '\n') {
        return 0;
    }
    if (c == '\n') {
        data->skip_line = FALSE;
        return 0;
    }
    if (c == '|') {
        data->skip_line = TRUE;
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

    if ((c >= '0' && c <= '9')
            || (c >= 'a' && c <= 'f')
            || (c >= 'A' && c <= 'F')) {
        data->prev_byte = c;
        return 0;
    }
    if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
        return 0;
    }
    return 1;
}

/* Return values: 0 for success, 1 for retry as to_hex, 2 for error */
static int try_from_hex(FILE *input_file,
        FILE *output_file,
        char *from_hex_read_buffer,
        size_t *from_hex_read_buffer_length,
        BOOL check_header) {
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
            fprintf(stderr,
                "Error: invalid format at character %llu, line %llu, "
                "col %llu, aborting\n",
                i, line_no, col_no);
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
        const char *from_hex_read_buffer,
        size_t from_hex_read_buffer_length) {
    uint64_t addr, output_data_len;
    size_t i;

    /* BLOCK_BATCH describes the number of blocks (sets of 16 bytes)
        to convert at once. Each block produces 81 bytes of output. */
    enum { BLOCK_BATCH = 64 };

    char output[81 * BLOCK_BATCH];
    char input[16 * BLOCK_BATCH];

    if (output_file) {
        fputs(header, output_file);
        fputc('\n', output_file);
    }
    
    addr = 0;
    
    /* read in chars already read in by try_from_hex */
    memcpy(input, from_hex_read_buffer, from_hex_read_buffer_length);
    
    for (;;) {
        i = from_hex_read_buffer_length
            + fread(input + from_hex_read_buffer_length,
                    1,
                    16 * (size_t)BLOCK_BATCH
                        - from_hex_read_buffer_length,
                    input_file);
        from_hex_read_buffer_length = 0;
        output_data_len = bin_data_to_hex(input, i, addr, output);
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
            fprintf(stderr,
                "Unable to open file `%s` for reading: %s\n",
                input_filename, strerror(errno));
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
                fprintf(stderr,
                    "Unable to open file `%s` for writing: %s\n",
                    output_filename, strerror(errno));
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
    ParseArgsResult parse_args_result;
    const char *input_filename;
    const char *output_filename;
    FILE *input_file, *output_file;
    char output_filename_buffer[TEMP_FILENAME_SIZE];
    size_t from_hex_read_buffer_length;
    char from_hex_read_buffer[HEADER_LENGTH];
    int res;

    parse_args_result = parse_args(argc, argv);
    if (parse_args_result.exit_with_error) {
        return parse_args_result.exit_with_error;
    } else if (parse_args_result.exit_with_success) {
        return EXIT_SUCCESS;
    }

    input_filename = parse_args_result.input_filename;
    output_filename = parse_args_result.output_filename;

    output_filename_buffer[0] = '\0';
    if (open_files(&input_file, &output_file,
            input_filename, output_filename,
            output_filename_buffer)) {
        return StatusCodeFailedToOpenFiles;
    }
    
    /* store read characters in case we need to retry as to_hex. */
    from_hex_read_buffer_length = 0;
    memset(from_hex_read_buffer, 0, HEADER_LENGTH);

    if (parse_args_result.conversion == ConversionOnlyEncode) {
        goto try_encode;
    }
    
    res = try_from_hex(
        input_file, output_file,
        from_hex_read_buffer, &from_hex_read_buffer_length,
        parse_args_result.conversion == ConversionAutoDetect);
    switch (res) {
        case 0: /* success */
            if (cleanup_files(input_file, output_file,
                    output_filename_buffer, output_filename)) {
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
            res = try_to_hex(input_file, output_file,
                from_hex_read_buffer, from_hex_read_buffer_length);
            if (res == 0) {
                if (cleanup_files(input_file, output_file,
                        output_filename_buffer, output_filename)) {
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
