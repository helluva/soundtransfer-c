
#include "simplebinary.h"

// returns an "array" of length 8
int* freq_8_from_input(unsigned char one_byte) {
    static int a[8];
    for (int i = 0; i < 8; ++i) {
        a[i] = 400 + (unsigned char) ((one_byte >> (7 - i)) & 0x1) * 400;
    }
    return a;
}

// returns an "array" of length 4
int* freq_4_from_input(unsigned char one_byte) {
    static int a[4];
    for (int i = 0; i < 4; ++i) {
        a[i] = 300 + (unsigned char) ((one_byte >> (6 - (i * 2))) & 0x3) * 200;
    }
    return a;
}

int* freq_2_from_input(char one_byte) {
    static int a[2];
    for (int i = 0; i < 2; ++i) {
        a[i] = ((one_byte >> (4 - (i * 4))) & 0xF);
    }
    return a;
}

