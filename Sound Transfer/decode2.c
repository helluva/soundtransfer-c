
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


static int received_freq_index = 0;

static int appended_bits_count = 0;



static int det_counter = 0;


static int moving_avg_index = 0;
static double moving_avg[SAMPLES_PER_CHUNK];




void initialize_decoder(int* num_of_tones, unsigned char** decoded_bytes) {

    num_of_tones_for_data = num_of_tones;
    decoded_bytes_p = decoded_bytes;

    header_chunk_count = 0;
    header_chunk = 0;

    det_counter = SAMPLES_PER_CHUNK;

    rolling_avg_index = 0;
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

    det_counter++;

    moving_avg[moving_avg_index % SAMPLES_PER_CHUNK] = frequency;
    double avg = find_avg(moving_avg);
    int det = close_frequency(avg);

    if (det && det_counter >= SAMPLES_PER_CHUNK / 2) {
        det_counter = 0;
        process(det);
    }

    return status;
}

void process(int frequency) {

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
    double difference = fabs(frequency - target_frequency);
    return difference < MATCH_THRESHOLD;
}
