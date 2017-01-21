
#include "simplebinary.h"

// returns an "array" of length 8
int* freq_from_input(char one_byte) {
    int[8] a;
    for (int i = 0; i < 8; ++i) {
        a[i] = 400 + ((one_byte >> (7 - i)) & 0x1) * 400;
    }
    return a;
}
