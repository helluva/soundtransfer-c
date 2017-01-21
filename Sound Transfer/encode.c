
#include <stdlib.h>

#include "globals.h"
#include "encode.h"

int* freqs_from_input(char* data, int num_of_bytes) {
    int* output = malloc(sizeof(int) * num_of_bytes * 8 / BITS_PER_TONE);
    for (int b = 0; b < num_of_bytes; ++b) {
        for (int i = 0; i < 8 / BITS_PER_TONE; ++i) {
            output[b * (8 / BITS_PER_TONE) + i] = BASE_FREQ
                + (unsigned char) (

//                   [cur byte ]    [          section of byte              ]    [         bit mask               ]
                    ((*data + b) >> (8 - BITS_PER_TONE - (i * BITS_PER_TONE))) & ((0x1 << (BITS_PER_TONE + 1)) - 1)

                ) * LINEAR_INTERVAL;
        }
    }
    return output;
}
