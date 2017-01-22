
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


static int appended_bits_count = 0;


static const int COUNTDOWN_SIZE = 6;
int candidate_freqs[SAMPLES_PER_CHUNK * 2];
static int candidate_countdown;



void initialize_decoder(int* num_of_tones, unsigned char** decoded_bytes) {

    num_of_tones_for_data = num_of_tones;
    decoded_bytes_p = decoded_bytes;

    header_chunk_count = 0;
    header_chunk = 0;

    det_counter = SAMPLES_PER_CHUNK;

    moving_avg_index = 0;
    for (int i = 0; i < SAMPLES_PER_CHUNK; ++i) {
        moving_avg[i] = 0;
    }

    appended_bits_count = 0;

    status = WAITING_FOR_START_FREQUENCY;
}


int receive_frame(double frequency) {

    if (status == UNINITIALIZED || status == TRANSFER_COMPLETE) {
        return status;
    }

    if (candidate_countdown <= 0) {
        int close = close_frequency(frequency);
        if (close != 0 && (close != candidate_freqs[0] || candidate_countdown < COUNTDOWN_SIZE - SAMPLES_PER_CHUNK)) {
            process(median(candidate_freqs, COUNTDOWN_SIZE - candidate_countdown + 1));
            candidate_freqs[0] = close;
            candidate_countdown = COUNTDOWN_SIZE;
        }
    } else {
        candidate_freqs[COUNTDOWN_SIZE - candidate_countdown];
    }
    candidate_countdown--;

    return status;
}

void process(int frequency) {

    printf("processing %i\n", frequency);

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



int cmpfunc (const void * a, const void * b) {
    return ( *(int*)a - *(int*)b );
}

double median(int* freqs, int len) {
    int temp[len];
    for (int i = 0; i < len; ++i) {
        temp[i] = freqs[i];
    }
    qsort(temp, len, sizeof(int), cmpfunc);
    return temp[len / 2];
}


int compare_freq(double frequency, double target_frequency) {
    double difference = fabs(frequency - target_frequency);
    return difference < MATCH_THRESHOLD;
}



int close_frequency(double freq) {
    for (int i = 0; i < (0x1 << BITS_PER_TONE); ++i) {
        if (compare_freq(freq, BASE_FREQ + i * LINEAR_INTERVAL)) {
            return BASE_FREQ + i * LINEAR_INTERVAL;
        }
    }
    if (compare_freq(freq, GUARD_FREQUENCY)) return GUARD_FREQUENCY;
    if (compare_freq(freq, GUARD_FREQUENCY_B)) return GUARD_FREQUENCY_B;
    return 0;
}
