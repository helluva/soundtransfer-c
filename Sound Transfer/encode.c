
#include <stdlib.h>

#include "globals.h"
#include "encode.h"

int* freqs_from_input(const char* data, int num_of_bytes) {
    int* output = malloc(sizeof(int) * (2 + num_of_bytes * 8 / BITS_PER_TONE));
    for (int b = 0; b < num_of_bytes; ++b) {
        for (int i = 0; i < 8 / BITS_PER_TONE; ++i) {
            output[2 + b * (8 / BITS_PER_TONE) + i] = BASE_FREQ
                + ((unsigned char) (

//                                    [cur byte ]     [          section of byte              ]                      [      bit mask            ]
                    (((unsigned char) *(data + b)) >> (8 - BITS_PER_TONE - (i * BITS_PER_TONE))) & (((unsigned char) (0x1 << BITS_PER_TONE)) - 1)

                )) * LINEAR_INTERVAL;
        }
    }
    output[0] = GUARD_FREQUENCY;
    output[1] = GUARD_FREQUENCY;
    return output;
}
