#include "args.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef HEXTOGGLE_VERSION
#define HEXTOGGLE_VERSION 1.0.6
#endif

#define STR_VALUE(arg) #arg
#define STR_VALUE_2(name) STR_VALUE(name)
#define VERSION_STR STR_VALUE_2(HEXTOGGLE_VERSION)

/* main usage string, but doesn't include return codes */
static const char *USAGE_STRING =
"hextoggle " VERSION_STR "\n"
"\n"
"Usage: hextoggle [file]            # toggle file in-place\n"
"       hextoggle [input] [output]  # read `input`, write to `output`\n"
"       hextoggle -                 # read from stdin/write to stdout\n"
"\n"
"Options:\n"
"       -d  --decode   # force decode (i.e. hex -> binary)\n"
"       -e  --encode   # force encode (i.e. binary -> hex)\n"
"       -h  --help     # show this usage information\n"
"       -n  --dry-run  # discard results\n"
"       -v  --verbose  # enable verbose output\n"
"       -V  --version  # show version number and quit\n"
"\n";

static void print_help_screen(FILE *file) {
    fprintf(file, "%s", USAGE_STRING);
    fprintf(file, "Return codes:\n");
    fprintf(file, "  %i   success\n", EXIT_SUCCESS);
    fprintf(file, "  %i   invalid arguments\n",
        StatusCodeInvalidArgs);
    fprintf(file, "  %i   failed to open input files\n",
        StatusCodeFailedToOpenFiles);
    fprintf(file, "  %i   failed to clean up files\n",
        StatusCodeFailedCleanup);
    fprintf(file, "  %i   invalid input\n",
        StatusCodeInvalidInput);
    fprintf(file, "  %i   internal assertion failed\n",
        StatusCodeAssertionFailed);
}

Args parse_args(int argc, const char *argv[]) {
    Args result;
    BOOL help_arg, dry_run, valid_args, raw_args, version_arg;
    int i;

    enum {
        MainArgStepInputFile = 0,
        MainArgStepOutputFile = 1,
        MainArgStepDone = 2
    } main_arg_step = MainArgStepInputFile;

    result.exit_with_error = 0;
    result.exit_with_success = 0;
    result.verbose = FALSE;
    result.conversion = ConversionAutoDetect;
    result.input_kind = InputKindStdio;
    result.input_filename = NULL;
    result.output_kind = OutputKindStdio;
    result.output_filename = NULL;

    help_arg = FALSE;
    version_arg = FALSE;
    dry_run = FALSE;
    valid_args = TRUE;
    raw_args = FALSE;
    for (i = 1; i < argc; ++i) {
        if (!valid_args) {
            continue;
        } else if (raw_args || strncmp("-", argv[i], 1)) {
            if (main_arg_step == MainArgStepInputFile) {
                result.input_filename = argv[i];
                result.input_kind = InputKindFileName;
                result.output_filename = argv[i];
                result.output_kind = OutputKindFileName;
                main_arg_step = MainArgStepOutputFile;
            } else if (main_arg_step == MainArgStepOutputFile) {
                result.output_filename = argv[i];
                result.output_kind = OutputKindFileName;
                main_arg_step = MainArgStepDone;
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
        } else if (!strcmp(argv[i], "--verbose")
                || !strcmp(argv[i], "-v")) {
            result.verbose = TRUE;
        } else if (!strcmp(argv[i], "--version")
                || !strcmp(argv[i], "-V")) {
            version_arg = TRUE;
        } else if (!strcmp(argv[i], "-")) {
            if (main_arg_step == MainArgStepInputFile) {
                result.input_kind = InputKindStdio;
                result.output_kind = OutputKindStdio;
                main_arg_step = MainArgStepOutputFile;
            } else if (main_arg_step == MainArgStepOutputFile) {
                result.output_kind = OutputKindStdio;
                main_arg_step = MainArgStepDone;
            } else {
                /* too many args */
                valid_args = FALSE;
            }
        } else if (!strcmp(argv[i], "--")) {
            raw_args = TRUE;
        } else {
            /* unknown argument */
            valid_args = FALSE;
        }
    }

    if (main_arg_step == MainArgStepInputFile
            && result.conversion == ConversionAutoDetect
            && !help_arg
            && !version_arg) {
        valid_args = FALSE;
    }

    if (dry_run) {
        result.output_kind = OutputKindNone;
    }

    if (!valid_args) {
        print_help_screen(stderr);
        result.exit_with_error = StatusCodeInvalidArgs;
    } else if (help_arg) {
        print_help_screen(stdout);
        result.exit_with_success = 1;
    } else if (version_arg) {
        printf("%s\n", "hextoggle " VERSION_STR);
        result.exit_with_success = 1;
    }

    return result;
}
