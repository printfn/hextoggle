#include "bin_to_hex.h"

#include "utils.h"

#include <stdbool.h>

static char char_to_hex(char input, bool first) {
    if (first) {
        return int_to_hex_char((int)((input >> 4) & 0xF));
    } else {
        return int_to_hex_char((int)(input & 0xF));
    }
}

/** Convert (up to) 16 bytes of binary data
 *      to (up to) 81 bytes of output.
 * `input` needs to point to `input_size` bytes of data (up to 16),
 * `addr` describes the overall offset in the input file, and `output`
 *      needs to point to 81 bytes of writable space. */
static size_t bin_block_to_hex(
        const char *input,
        size_t input_size,
        uint64_t addr,
        char *output) {
    output[0] = '[';
    output[1] = int_to_hex_char((addr >> 36) & 0xF);
    output[2] = int_to_hex_char((addr >> 32) & 0xF);
    output[3] = int_to_hex_char((addr >> 28) & 0xF);
    output[4] = int_to_hex_char((addr >> 24) & 0xF);
    output[5] = int_to_hex_char((addr >> 20) & 0xF);
    output[6] = int_to_hex_char((addr >> 16) & 0xF);
    output[7] = int_to_hex_char((addr >> 12) & 0xF);
    output[8] = int_to_hex_char((addr >> 8) & 0xF);
    output[9] = int_to_hex_char((addr >> 4) & 0xF);
    output[10] = int_to_hex_char(addr & 0xF);
    output[11] = ' ';
    output[12] = '0' + addr / 10000000000 % 10;
    output[13] = '0' + addr / 1000000000 % 10;
    output[14] = '0' + addr / 100000000 % 10;
    output[15] = '0' + addr / 10000000 % 10;
    output[16] = '0' + addr / 1000000 % 10;
    output[17] = '0' + addr / 100000 % 10;
    output[18] = '0' + addr / 10000 % 10;
    output[19] = '0' + addr / 1000 % 10;
    output[20] = '0' + addr / 100 % 10;
    output[21] = '0' + addr / 10 % 10;
    output[22] = '0' + addr % 10;
    output[23] = ']';
    output[24] = 0 < input_size ? char_to_hex(input[0], true) : ' ';
    output[25] = 0 < input_size ? char_to_hex(input[0], false) : ' ';
    output[26] = 1 < input_size ? char_to_hex(input[1], true) : ' ';
    output[27] = 1 < input_size ? char_to_hex(input[1], false) : ' ';
    output[28] = ' ';
    output[29] = 2 < input_size ? char_to_hex(input[2], true) : ' ';
    output[30] = 2 < input_size ? char_to_hex(input[2], false) : ' ';
    output[31] = 3 < input_size ? char_to_hex(input[3], true) : ' ';
    output[32] = 3 < input_size ? char_to_hex(input[3], false) : ' ';
    output[33] = ' ';
    output[34] = 4 < input_size ? char_to_hex(input[4], true) : ' ';
    output[35] = 4 < input_size ? char_to_hex(input[4], false) : ' ';
    output[36] = 5 < input_size ? char_to_hex(input[5], true) : ' ';
    output[37] = 5 < input_size ? char_to_hex(input[5], false) : ' ';
    output[38] = ' ';
    output[39] = 6 < input_size ? char_to_hex(input[6], true) : ' ';
    output[40] = 6 < input_size ? char_to_hex(input[6], false) : ' ';
    output[41] = 7 < input_size ? char_to_hex(input[7], true) : ' ';
    output[42] = 7 < input_size ? char_to_hex(input[7], false) : ' ';
    output[43] = ' ';
    output[44] = 8 < input_size ? char_to_hex(input[8], true) : ' ';
    output[45] = 8 < input_size ? char_to_hex(input[8], false) : ' ';
    output[46] = 9 < input_size ? char_to_hex(input[9], true) : ' ';
    output[47] = 9 < input_size ? char_to_hex(input[9], false) : ' ';
    output[48] = ' ';
    output[49] = 10 < input_size ? char_to_hex(input[10], true) : ' ';
    output[50] = 10 < input_size ? char_to_hex(input[10], false) : ' ';
    output[51] = 11 < input_size ? char_to_hex(input[11], true) : ' ';
    output[52] = 11 < input_size ? char_to_hex(input[11], false) : ' ';
    output[53] = ' ';
    output[54] = 12 < input_size ? char_to_hex(input[12], true) : ' ';
    output[55] = 12 < input_size ? char_to_hex(input[12], false) : ' ';
    output[56] = 13 < input_size ? char_to_hex(input[13], true) : ' ';
    output[57] = 13 < input_size ? char_to_hex(input[13], false) : ' ';
    output[58] = ' ';
    output[59] = 14 < input_size ? char_to_hex(input[14], true) : ' ';
    output[60] = 14 < input_size ? char_to_hex(input[14], false) : ' ';
    output[61] = 15 < input_size ? char_to_hex(input[15], true) : ' ';
    output[62] = 15 < input_size ? char_to_hex(input[15], false) : ' ';
    output[63] = '|';
    output[64] = 0 < input_size ? safe_char(input[0]) : ' ';
    output[65] = 1 < input_size ? safe_char(input[1]) : ' ';
    output[66] = 2 < input_size ? safe_char(input[2]) : ' ';
    output[67] = 3 < input_size ? safe_char(input[3]) : ' ';
    output[68] = 4 < input_size ? safe_char(input[4]) : ' ';
    output[69] = 5 < input_size ? safe_char(input[5]) : ' ';
    output[70] = 6 < input_size ? safe_char(input[6]) : ' ';
    output[71] = 7 < input_size ? safe_char(input[7]) : ' ';
    output[72] = 8 < input_size ? safe_char(input[8]) : ' ';
    output[73] = 9 < input_size ? safe_char(input[9]) : ' ';
    output[74] = 10 < input_size ? safe_char(input[10]) : ' ';
    output[75] = 11 < input_size ? safe_char(input[11]) : ' ';
    output[76] = 12 < input_size ? safe_char(input[12]) : ' ';
    output[77] = 13 < input_size ? safe_char(input[13]) : ' ';
    output[78] = 14 < input_size ? safe_char(input[14]) : ' ';
    output[79] = 15 < input_size ? safe_char(input[15]) : ' ';
    output[80] = '\n';
    size_t output_size = 81;
    if (input_size < 16) {
        output_size -= (16 - input_size);
        output[output_size - 1] = '\n';
    }
    return output_size;
}

size_t bin_data_to_hex(
        const char *input,
        size_t input_size,
        uint64_t addr,
        char *output) {
    size_t output_size = 0;
    size_t input_offset = 0;
    while (input_offset < input_size) {
        size_t block_size = 16;
        if (input_size - input_offset < 16) {
            block_size = input_size - input_offset;
        }
        output_size += bin_block_to_hex(
            input, block_size, addr, output + output_size);
        input += 16;
        input_offset += 16;
        addr += 16;
    }
    return output_size;
}
