/* Convert a hexadecimal character (i.e. [0-9a-fA-F]) to an integer
value between 0 and 15 inclusive. Returns -1 on error. */
int hex_char_to_int(char ch);

/* Convert a number between 0 and 15 (inclusive) to a hex digit.
Returns '\0' on error. */
char int_to_hex_char(int number);

/* Returns a 'safe' version of the given character.
All control/formatting characters will be replaced with '.' */
char safe_char(char ch);

/* Custom bool type */
typedef enum {
    FALSE = 0,
    TRUE = 1
} BOOL;

enum StatusCode {
    StatusCodeInvalidArgs = 1,
    StatusCodeFailedToOpenFiles,
    StatusCodeFailedCleanup,
    StatusCodeInvalidInput,
    StatusCodeAssertionFailed
};
