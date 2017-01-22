
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

    chunk = malloc(sizeof(double) * SAMPLES_PER_CHUNK);

    status = WAITING_FOR_START_FREQUENCY;
}

int receive_frame(double frequency) {

    if (status == UNINITIALIZED || status == TRANSFER_COMPLETE) {
        return status;
    }

    //printf("%i\n", (int)frequency);

    chunk[received_freq_index++] = frequency;
    if (received_freq_index == SAMPLES_PER_CHUNK) {
        process_chunk(chunk);
        received_freq_index = 0;
    }

    return status;
}

void process_chunk(double* chunk) {

    //printf("-200\n");

    int num_of_frequencies = (0x1 << BITS_PER_TONE);
    int num_of_buckets = num_of_frequencies + 2;
    int GUARD_FREQUENCY_TEXT_bucket = num_of_frequencies;
    int GUARD_FREQUENCY_TEXT_B_bucket = num_of_frequencies + 1;

    int matches_per_frequency[num_of_buckets];
    for (int i = 0; i < num_of_buckets; i++) {
        matches_per_frequency[i] = 0;
    }


    for (int i = 0; i < SAMPLES_PER_CHUNK; ++i) {

        if (compare_freq(chunk[i], GUARD_FREQUENCY_TEXT)) {
            matches_per_frequency[GUARD_FREQUENCY_TEXT_bucket]++;
        } else if (compare_freq(chunk[i], GUARD_FREQUENCY_TEXT_B)) {
            matches_per_frequency[GUARD_FREQUENCY_TEXT_B_bucket]++;
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
        if (max_count_index == GUARD_FREQUENCY_TEXT_bucket) {
            process_tone(GUARD_FREQUENCY_TEXT);
        } else if (max_count_index == GUARD_FREQUENCY_TEXT_B_bucket) {
            process_tone(GUARD_FREQUENCY_TEXT_B);
        }  else {
            process_tone(BASE_FREQ + LINEAR_INTERVAL * max_count_index);
        }
    }

}

void process_tone(int frequency) {

    //printf("received %i\n", frequency);

    if (status == WAITING_FOR_START_FREQUENCY && frequency == GUARD_FREQUENCY_TEXT) {
        status = DETECTED_START_FREQUENCY;
    } else if (status == DETECTED_START_FREQUENCY) {
        status = WAITING_FOR_START_FREQUENCY;
        if (frequency == GUARD_FREQUENCY_TEXT_B) {
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

        printf("%i\n", frequency);

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

int* process_colors(unsigned char bits) {
    static int a[4];
    for (int i = 0; i < 4; ++i) {
        a[i] = (bits >> (2 * (3 - i))) & 0x3;
    }
    return a;
}


int compare_freq(double frequency, double target_frequency) {
    double difference = fabs(frequency - target_frequency);
    return difference < MATCH_THRESHOLD;
}
