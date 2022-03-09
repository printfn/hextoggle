#include "utils.h"

#include <stdio.h>

int hex_char_to_int(char ch) {
    switch (ch) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'a': case 'A': return 10;
        case 'b': case 'B': return 11;
        case 'c': case 'C': return 12;
        case 'd': case 'D': return 13;
        case 'e': case 'E': return 14;
        case 'f': case 'F': return 15;
        default: break;
    }
    fprintf(stderr,
        "Internal error: invalid hex character `%c`\n", ch);
    return -1;
}

char int_to_hex_char(int number) {
    switch (number) {
        case 0: return '0';
        case 1: return '1';
        case 2: return '2';
        case 3: return '3';
        case 4: return '4';
        case 5: return '5';
        case 6: return '6';
        case 7: return '7';
        case 8: return '8';
        case 9: return '9';
        case 10: return 'a';
        case 11: return 'b';
        case 12: return 'c';
        case 13: return 'd';
        case 14: return 'e';
        case 15: return 'f';
        default: break;
    }
    fprintf(stderr,
        "Internal error: cannot convert `%d` to a hex digit\n",
        number);
    return '\0';
}

char safe_char(char ch) {
    if (ch >= ' ' && ch <= '~') {
        return ch;
    } else {
        return '.';
    }
}
