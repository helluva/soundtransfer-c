
#include <stdlib.h>
#include <stdio.h>
#include "globals.h"
#include "encode.h"


int* freqs_from_color(const char* data, int num_colors) {
    
    char* mid = malloc(num_colors / 4);
    
    for (int i = 0; i < num_colors; i += 4) {
        mid[i / 4] = (data[i] << 6) | (data[i + 1] << 4) | (data[i + 2] << 2) | data[i + 3];
    }
    
    return freqs_from_input(mid, (num_colors / 4));
}

int* freqs_from_input(const char* data, int num_of_bytes) {
    
    int mallocSize = (5 + num_of_bytes * 8 / BITS_PER_TONE);
    int* output = malloc(sizeof(int) * mallocSize);
    for (int b = 0; b < num_of_bytes; ++b) {
        for (int i = 0; i < 8 / BITS_PER_TONE; ++i) {
            output[4 + b * (8 / BITS_PER_TONE) + i] = BASE_FREQ
                + ((unsigned char) (

//                                    [cur byte ]     [          section of byte              ]                      [      bit mask            ]
                    (((unsigned char) *(data + b)) >> (8 - BITS_PER_TONE - (i * BITS_PER_TONE))) & (((unsigned char) (0x1 << BITS_PER_TONE)) - 1)

                )) * LINEAR_INTERVAL;
        }
    }

    int x = BASE_FREQ
        + ((unsigned char) (

//                                    [cur byte ]     [          section of byte              ]                      [      bit mask            ]
            (((unsigned char) (num_of_bytes)) >> (8 - BITS_PER_TONE - (0 * BITS_PER_TONE))) & (((unsigned char) (0x1 << BITS_PER_TONE)) - 1)

        )) * LINEAR_INTERVAL;

    int y = BASE_FREQ
        + ((unsigned char) (

//                                    [cur byte ]     [          section of byte              ]                      [      bit mask            ]
            (((unsigned char) (num_of_bytes)) >> (8 - BITS_PER_TONE - (1 * BITS_PER_TONE))) & (((unsigned char) (0x1 << BITS_PER_TONE)) - 1)

        )) * LINEAR_INTERVAL;
    
    output[0] = GUARD_FREQUENCY_TEXT;
    output[1] = GUARD_FREQUENCY_TEXT_B;
    output[2] = x;
    output[3] = y;
    output[mallocSize - 1] = GUARD_FREQUENCY_TEXT;
    return separate_repeating_freqs(output, mallocSize);
}

int* separate_repeating_freqs(int* freqs, int num_of_freqs) {
    
    //determine extra space needed for new malloc
    int numberOfSeparatorsNeeded = 0;
    int previousFrequency= -1;
    
    for (int i = 0; i < num_of_freqs; i++) {
        int freq = freqs[i];
        if (freq == previousFrequency) {
            numberOfSeparatorsNeeded += 1;
        }
        previousFrequency = freq;
    }
    
    //malloc new space
    int newCount = num_of_freqs + numberOfSeparatorsNeeded;
    int* newFreqs = malloc(sizeof(int) * (newCount));
    
    //add all freqs to the new array with separators
    int numberOfSeparatorsUsedSoFar = 0;
    previousFrequency = -1;
    
    for (int i = 0; i < newCount; i++) {
        int freq = freqs[i - numberOfSeparatorsUsedSoFar];
        
        if (freq == previousFrequency) {
            numberOfSeparatorsUsedSoFar += 1;
            newFreqs[i] = REPEAT_SEPARATOR_FREQUENCY;
            previousFrequency = -1;
        } else {
            newFreqs[i] = freq;
            previousFrequency = freq;
        }
    }
    
    return newFreqs;
    
}
