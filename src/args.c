#include "args.h"

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(HEXTOGGLE_VERSION)
/* be careful not to indent the line after the backslash */
#error Make sure `HEXTOGGLE_VERSION` is set to the \
correct version number (e.g. `1.0.0`)
#endif
#define STR_VALUE(arg) #arg
#define STR_VALUE_2(name) STR_VALUE(name)
#define VERSION_STR STR_VALUE_2(HEXTOGGLE_VERSION)

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

Args parse_args(int argc, const char *argv[]) {
    Args result;
    BOOL help_arg, dry_run, valid_args, raw_args;
    int i;

    enum {
        MainArgStepInputFile = 0,
        MainArgStepOutputFile = 1,
        MainArgStepDone = 2
    } main_arg_step = MainArgStepInputFile;

    result.exit_with_error = 0;
    result.exit_with_success = 0;
    result.conversion = ConversionAutoDetect;
    result.input_kind = InputKindStdio;
    result.input_filename = NULL;
    result.output_kind = OutputKindStdio;
    result.output_filename = NULL;

    help_arg = FALSE;
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
            && result.conversion == ConversionAutoDetect) {
        valid_args = FALSE;
    }

    if (dry_run) {
        result.output_kind = OutputKindNone;
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
