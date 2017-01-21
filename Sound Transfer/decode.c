
#include "globals.h"
#include "decode.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static const int UNINITIALIZED = -99;
static const int WAITING_FOR_START_FREQUENCY = -3;
static const int DETECTED_START_FREQUENCY = -4;
static const int RECEIVING_HEADER = -2;
static const int RECEIVING_BODY = -5;
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
    *num_of_tones_for_data = 54;
    *decoded_bytes_p = malloc(sizeof(char) * 27); // 4 bytes
    // THIS IS TEMPORARY!


    chunk = malloc(sizeof(double) * SAMPLES_PER_CHUNK);

    status = WAITING_FOR_START_FREQUENCY;
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

    int matches_per_frequency[num_of_frequencies + 1];
    for (int i = 0; i < num_of_frequencies + 1; i++) {
        matches_per_frequency[i] = 0;
    }
    

    for (int i = 0; i < SAMPLES_PER_CHUNK; ++i) {

        if (compare_freq(chunk[i], GUARD_FREQUENCY)) {
            matches_per_frequency[num_of_frequencies]++;
            printf("matches for guard: %i\n", matches_per_frequency[num_of_frequencies]);
        }
        
        else {
            for (int f = 0; f < num_of_frequencies; ++f) {
                if (compare_freq(chunk[i], BASE_FREQ + LINEAR_INTERVAL * f)) {
                    matches_per_frequency[f]++;
                }
            }
        }

    }

    int max_count = 0;
    int max_count_index = 0;
    for (int f = 0; f < num_of_frequencies + 1; ++f) {
        if (matches_per_frequency[f] > max_count) {
            max_count = matches_per_frequency[f];
            max_count_index = f;
        }
    }

    if (max_count > 3) {
        if (max_count_index == num_of_frequencies) {
            process_tone(GUARD_FREQUENCY);
        } else {
            process_tone(BASE_FREQ + LINEAR_INTERVAL * max_count_index);
        }
    }
    

}

void process_tone(int frequency) {

    // process intro tone and length data here

    printf("\n%i // %i\n", status, frequency);
    
    if (status == WAITING_FOR_START_FREQUENCY && frequency == GUARD_FREQUENCY) {
        status = DETECTED_START_FREQUENCY;
    } else if (status == DETECTED_START_FREQUENCY && frequency == GUARD_FREQUENCY) {
        status = RECEIVING_BODY;
    } else if (status == RECEIVING_BODY) {
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
