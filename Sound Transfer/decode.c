
#include "globals.h"
#include "decode.h"
#include <math.h>
#include <stdlib.h>


static const int UNINITIALIZED = -99;
static const int NOT_RECEIVING = -1;
static const int GUARD = -3;
static const int RECEIVING_HEADER = -2;
static const int TRANSFER_COMPLETE = 0;

static int status = UNINITIALIZED;

static int* num_of_tones_for_data;
static unsigned char** decoded_bytes_p;


static double* chunk;
static int received_freq_index = 0;

static int appended_bits_count = 0;


void initialize_decoder(int* num_of_tones, unsigned char** decoded_bytes) {

    num_of_tones_for_data = num_of_tones;
    decoded_bytes_p = decoded_bytes;

    appended_bits_count = 0;


    // THIS IS TEMPORARY FOR TESTING!
    *num_of_tones_for_data = 8;
    *decoded_bytes_p = malloc(sizeof(char) * 4); // 4 bytes
    // THIS IS TEMPORARY!


    chunk = malloc(sizeof(double) * SAMPLES_PER_CHUNK);

    status = NOT_RECEIVING;
}

int receive_frame(double frequency) {

    if (status == UNINITIALIZED) {
        return status;
    }

    chunk[received_freq_index++] = frequency;
    if (received_freq_index == SAMPLES_PER_CHUNK) {
        process_chunk(chunk);
        received_freq_index = 0;
    }

    return status;
}

void process_chunk(double* chunk) {

    int num_of_frequencies = 0x1 << BITS_PER_TONE;

    int matches_per_frequency[num_of_frequencies];

    for (int i = 0; i < SAMPLES_PER_CHUNK; ++i) {

        for (int f = 0; f < num_of_frequencies; ++f) {
            if (compare_freq(chunk[i], BASE_FREQ + LINEAR_INTERVAL * f)) {
                matches_per_frequency[f]++;
            }
        }

    }

    int max_count = 0;
    int max_count_index = 0;
    for (int f = 0; f < num_of_frequencies; ++f) {
        if (matches_per_frequency[f] >= max_count) {
            max_count = matches_per_frequency[f];
            max_count_index = f;
        }
    }

    process_tone(BASE_FREQ + LINEAR_INTERVAL * max_count_index);

}

void process_tone(int frequency) {

    // process intro tone and length data here

    if (status != GUARD && frequency == GUARD_FREQUENCY) {
        status = GUARD;
    } else if (frequency == GUARD_FREQUENCY) {
    } else {
        append_bits((unsigned char) ((frequency - BASE_FREQ) / LINEAR_INTERVAL));
    }

}

void append_bits(unsigned char bits) {

    unsigned char* cur_byte = &((*decoded_bytes_p)[appended_bits_count / 8]);

    *cur_byte += bits << (8 - appended_bits_count % 8 - BITS_PER_TONE);

    appended_bits_count += BITS_PER_TONE;

}

int compare_freq(double frequency, double target_frequency) {
    double difference = abs(frequency - target_frequency);
    return difference < MATCH_THRESHOLD;
}
