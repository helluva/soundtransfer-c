
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

static int header_chunk_count = 0;
static unsigned char header_chunk = 0;

static int* num_of_tones_for_data;
static unsigned char** decoded_bytes_p;


static double* chunk;
static int received_freq_index = 0;

static int appended_bits_count = 0;


void initialize_decoder(int* num_of_tones, unsigned char** decoded_bytes) {

    num_of_tones_for_data = num_of_tones;
    decoded_bytes_p = decoded_bytes;

    header_chunk_count = 0;
    header_chunk = 0;

    appended_bits_count = 0;


    // THIS IS TEMPORARY FOR TESTING!
    *num_of_tones_for_data = 54;
    *decoded_bytes_p = malloc(sizeof(char) * 27); // 4 bytes
    // THIS IS TEMPORARY!


    chunk = malloc(sizeof(double) * SAMPLES_PER_CHUNK);

    status = WAITING_FOR_START_FREQUENCY;
}

int receive_frame(double frequency) {

    if (status == UNINITIALIZED || status == TRANSFER_COMPLETE) {
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

    int num_of_frequencies = (0x1 << BITS_PER_TONE);
    int num_of_buckets = num_of_frequencies + 2;
    int guard_frequency_bucket = num_of_frequencies;
    int guard_frequency_B_bucket = num_of_frequencies + 1;

    int matches_per_frequency[num_of_buckets];
    for (int i = 0; i < num_of_buckets; i++) {
        matches_per_frequency[i] = 0;
    }


    for (int i = 0; i < SAMPLES_PER_CHUNK; ++i) {

        if (compare_freq(chunk[i], GUARD_FREQUENCY)) {
            matches_per_frequency[guard_frequency_bucket]++;
        } else if (compare_freq(chunk[i], GUARD_FREQUENCY_B)) {
            matches_per_frequency[guard_frequency_B_bucket]++;
        } else {
            for (int f = 0; f < num_of_frequencies; ++f) {
                if (compare_freq(chunk[i], BASE_FREQ + LINEAR_INTERVAL * f)) {
                    matches_per_frequency[f]++;
                }
            }
        }

    }

    int max_count = 0;
    int max_count_index = 0;
    for (int f = 0; f < num_of_buckets; ++f) {
        if (matches_per_frequency[f] > max_count) {
            max_count = matches_per_frequency[f];
            max_count_index = f;
        }
    }

    if (max_count > 3) {
        if (max_count_index == guard_frequency_bucket) {
            process_tone(GUARD_FREQUENCY);
        } else if (max_count_index == guard_frequency_B_bucket) {
            process_tone(GUARD_FREQUENCY_B);
        }  else {
            process_tone(BASE_FREQ + LINEAR_INTERVAL * max_count_index);
        }
    }

}

void process_tone(int frequency) {

    printf("\n%i // %i\n", status, frequency);

    if (status == WAITING_FOR_START_FREQUENCY && frequency == GUARD_FREQUENCY) {
        status = DETECTED_START_FREQUENCY;
    } else if (status == DETECTED_START_FREQUENCY) {
        status = WAITING_FOR_START_FREQUENCY;
        if (frequency == GUARD_FREQUENCY_B) {
            status = RECEIVING_HEADER;
        }
    } else if (status == RECEIVING_HEADER) {
        unsigned char bits = (unsigned char) ((frequency - BASE_FREQ) / LINEAR_INTERVAL);
        header_chunk += bits << (8 - BITS_PER_TONE - header_chunk_count * BITS_PER_TONE);
        header_chunk_count++;
        if (header_chunk_count >= 8 / BITS_PER_TONE) {
            *num_of_tones_for_data = header_chunk * (8 / BITS_PER_TONE);
            *decoded_bytes_p = malloc(sizeof(char) * header_chunk);
            status = RECEIVING_BODY;
        }
    } else if (status == RECEIVING_BODY) {
        append_bits((unsigned char) ((frequency - BASE_FREQ) / LINEAR_INTERVAL));
    }

}

void append_bits(unsigned char bits) {

    unsigned char* cur_byte = &((*decoded_bytes_p)[appended_bits_count / 8]);

    *cur_byte += bits << (8 - appended_bits_count % 8 - BITS_PER_TONE);

    appended_bits_count += BITS_PER_TONE;

    if (appended_bits_count / 8 >= *num_of_tones_for_data) {
        status = TRANSFER_COMPLETE;
    }

}




int compare_freq(double frequency, double target_frequency) {
    double difference = abs(frequency - target_frequency);
    return difference < MATCH_THRESHOLD;
}
