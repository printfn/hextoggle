#include <stdint.h>

/*
Format:
[0000000000 00000000000]4865 6c6c 6f2c 2057 6f72 6c64 210a 0a23|Hello, World!..#\n
0         1         2         3         4         5         6         7         8
012345678901234567890123456789012345678901234567890123456789012345678901234567890
*/

/// Convert binary data to hex. This function can take an arbitrary
/// amount of binary input in multiples of 16 (unless we're at the end
/// of input).
///
/// `input`: points to binary data (size must be a multiple of 16
///     unless near EOF)
/// `input_size`: size of the specified input
/// `addr`: address in overall data
/// `output`: space we can use for output, must be equal to
///     81 * ceil(input_size / 16) bytes
/// Return value: amount of data written to output
uint64_t bin_data_to_hex(
    const char *input,
    uint64_t input_size,
    uint64_t addr,
    char *output);