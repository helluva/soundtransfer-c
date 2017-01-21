
#include "globals.h"
#include "encode.h"

void freqs_from_input(char* data, int num_of_bytes, int* output) {
    for (int b = 0; b < num_of_bytes; ++b) {
        for (int i = 0; i < 8 / BITS_PER_TONE; ++i) {
            output[b * (8 / BITS_PER_TONE) + i] = BASE_FREQ + (unsigned char) (((*data + b) >> (8 - BITS_PER_TONE - (i * BITS_PER_TONE))) & ((0x1 << (BITS_PER_TONE + 1)) - 1)) * LINEAR_INTERVAL;
        }
    }
}
