
#include "encode.h"

const int BASE_FREQ = 1000;
const int LINEAR_INTERVAL = 100;

int* freq_2_from_input(char one_byte) {
    static int a[2];
    for (int i = 0; i < 2; ++i) {
        a[i] = BASE_FREQ + (unsigned char) ((one_byte >> (4 - (i * 4))) & 0xF) * LINEAR_INTERVAL;
    }
    return a;
}