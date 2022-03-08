#include "bin_to_hex.h"

#include "utils.h"

// Convert (up to) 16 bytes of binary data to 81 bytes of output.
// `input` needs to point to `input_size` bytes of data (up to 16),
// `addr` describes the overall offset in the input file, and `output`
// needs to point to 81 bytes of writable space.
static void bin_block_to_hex(
        const char *input,
        uint64_t input_size,
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
    output[24] = 0 < input_size ? int_to_hex_char((int)((input[0] >> 4) & 0xF)) : ' ';
    output[25] = 0 < input_size ? int_to_hex_char((int)(input[0] & 0xF)) : ' ';
    output[26] = 1 < input_size ? int_to_hex_char((int)((input[1] >> 4) & 0xF)) : ' ';
    output[27] = 1 < input_size ? int_to_hex_char((int)(input[1] & 0xF)) : ' ';
    output[28] = ' ';
    output[29] = 2 < input_size ? int_to_hex_char((int)((input[2] >> 4) & 0xF)) : ' ';
    output[30] = 2 < input_size ? int_to_hex_char((int)(input[2] & 0xF)) : ' ';
    output[31] = 3 < input_size ? int_to_hex_char((int)((input[3] >> 4) & 0xF)) : ' ';
    output[32] = 3 < input_size ? int_to_hex_char((int)(input[3] & 0xF)) : ' ';
    output[33] = ' ';
    output[34] = 4 < input_size ? int_to_hex_char((int)((input[4] >> 4) & 0xF)) : ' ';
    output[35] = 4 < input_size ? int_to_hex_char((int)(input[4] & 0xF)) : ' ';
    output[36] = 5 < input_size ? int_to_hex_char((int)((input[5] >> 4) & 0xF)) : ' ';
    output[37] = 5 < input_size ? int_to_hex_char((int)(input[5] & 0xF)) : ' ';
    output[38] = ' ';
    output[39] = 6 < input_size ? int_to_hex_char((int)((input[6] >> 4) & 0xF)) : ' ';
    output[40] = 6 < input_size ? int_to_hex_char((int)(input[6] & 0xF)) : ' ';
    output[41] = 7 < input_size ? int_to_hex_char((int)((input[7] >> 4) & 0xF)) : ' ';
    output[42] = 7 < input_size ? int_to_hex_char((int)(input[7] & 0xF)) : ' ';
    output[43] = ' ';
    output[44] = 8 < input_size ? int_to_hex_char((int)((input[8] >> 4) & 0xF)) : ' ';
    output[45] = 8 < input_size ? int_to_hex_char((int)(input[8] & 0xF)) : ' ';
    output[46] = 9 < input_size ? int_to_hex_char((int)((input[9] >> 4) & 0xF)) : ' ';
    output[47] = 9 < input_size ? int_to_hex_char((int)(input[9] & 0xF)) : ' ';
    output[48] = ' ';
    output[49] = 10 < input_size ? int_to_hex_char((int)((input[10] >> 4) & 0xF)) : ' ';
    output[50] = 10 < input_size ? int_to_hex_char((int)(input[10] & 0xF)) : ' ';
    output[51] = 11 < input_size ? int_to_hex_char((int)((input[11] >> 4) & 0xF)) : ' ';
    output[52] = 11 < input_size ? int_to_hex_char((int)(input[11] & 0xF)) : ' ';
    output[53] = ' ';
    output[54] = 12 < input_size ? int_to_hex_char((int)((input[12] >> 4) & 0xF)) : ' ';
    output[55] = 12 < input_size ? int_to_hex_char((int)(input[12] & 0xF)) : ' ';
    output[56] = 13 < input_size ? int_to_hex_char((int)((input[13] >> 4) & 0xF)) : ' ';
    output[57] = 13 < input_size ? int_to_hex_char((int)(input[13] & 0xF)) : ' ';
    output[58] = ' ';
    output[59] = 14 < input_size ? int_to_hex_char((int)((input[14] >> 4) & 0xF)) : ' ';
    output[60] = 14 < input_size ? int_to_hex_char((int)(input[14] & 0xF)) : ' ';
    output[61] = 15 < input_size ? int_to_hex_char((int)((input[15] >> 4) & 0xF)) : ' ';
    output[62] = 15 < input_size ? int_to_hex_char((int)(input[15] & 0xF)) : ' ';
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
}

uint64_t bin_data_to_hex(
        const char *input,
        uint64_t input_size,
        uint64_t addr,
        char *output) {
    uint64_t output_size = 0;
    uint64_t input_offset = 0;
    while (input_offset < input_size) {
        uint64_t block_size = 16;
        if (input_size - input_offset < 16) {
            block_size = input_size - input_offset;
        }
        bin_block_to_hex(input, block_size, addr, output);
        input += 16;
        input_offset += 16;
        addr += 16;
        output_size += 81;
        output += 81;
    }
    return output_size;
}
