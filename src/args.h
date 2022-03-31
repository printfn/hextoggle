typedef enum Conversion {
    ConversionAutoDetect,
    ConversionOnlyDecode,
    ConversionOnlyEncode
} Conversion;

/* describes whether we are using a file (with a filename) or using
stdin/stdout */
typedef enum InputKind {
    InputKindFileName,
    InputKindStdio
} InputKind;

typedef enum OutputKind {
    OutputKindFileName,
    OutputKindStdio,
    OutputKindNone /* used for --dry-run */
} OutputKind;

typedef struct {
    int exit_with_error;   /* if non-zero, exit with that error code */
    int exit_with_success; /* if non-zero, exit successfully */
    Conversion conversion;
    InputKind input_kind; /* opening a file vs. reading from stdin */
    const char *input_filename;
    OutputKind output_kind;
    const char *output_filename; /* null if we're doing a dry run */
} Args;

/** Validate the given command-line arguments,
and print usage description on error */
Args parse_args(int argc, const char *argv[]);
