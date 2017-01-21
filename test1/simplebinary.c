
#include "simplebinary.h"

// returns an "array" of length 8
int* freq_8_from_input(char one_byte) {
    static int[8] a;
    for (int i = 0; i < 8; ++i) {
        a[i] = 400 + ((one_byte >> (7 - i)) & 0x1) * 400;
    }
    return a;
}

// returns an "array" of length 4
int* freq_4_from_input(char one_byte) {
    static int[4] a;
    for (int i = 0; i < 4; ++i) {
        a[i] = 300 + ((one_byte >> (7 - (i * 2))) & 0x3) * 200;
    }
    return a;
}
