
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "globals.h"
#include "decode.h"

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


static int candidate_freq;



void initialize_decoder(int* num_of_tones, unsigned char** decoded_bytes) {

    num_of_tones_for_data = num_of_tones;
    decoded_bytes_p = decoded_bytes;

    header_chunk_count = 0;
    header_chunk = 0;

    appended_bits_count = 0;

    candidate_freq = 0;

    status = WAITING_FOR_START_FREQUENCY;
}


int receive_frame(double frequency) {

    printf("%i\n", (int)frequency);

    if (status == UNINITIALIZED || status == TRANSFER_COMPLETE) {
        return status;
    }

    int close = close_frequency(frequency);

    if ((close != 0 && close != candidate_freq) || (close == GUARD_FREQUENCY_TEXT)) {
        candidate_freq = close;
        process(candidate_freq);
    }

    return status;
}

void process(int frequency) {

    //printf("processing %i\n", frequency);

    if (status == WAITING_FOR_START_FREQUENCY && frequency == GUARD_FREQUENCY_TEXT) {
        status = DETECTED_START_FREQUENCY;
        //printf("status = detected start freq\n");
    } else if (status == DETECTED_START_FREQUENCY) {
        status = WAITING_FOR_START_FREQUENCY;
        if (frequency == GUARD_FREQUENCY_TEXT) {
            status = DETECTED_START_FREQUENCY;
        }
        //printf("status = waiting for start\n");
        if (frequency == GUARD_FREQUENCY_TEXT_B) {
            status = RECEIVING_HEADER;
            //printf("status = receiving header\n");
        }
    } else {

        if (frequency == REPEAT_SEPARATOR_FREQUENCY) {
            return;
        }

        if (status == RECEIVING_HEADER) {
            unsigned char bits = (unsigned char) ((frequency - BASE_FREQ) / LINEAR_INTERVAL);
            header_chunk += bits << (8 - BITS_PER_TONE - header_chunk_count * BITS_PER_TONE);
            header_chunk_count++;

            //printf("header chunk: %i     count: %i\n", (int)header_chunk, header_chunk_count);

            if (header_chunk_count >= 8 / BITS_PER_TONE) {
                *num_of_tones_for_data = header_chunk * (8 / BITS_PER_TONE);

                *decoded_bytes_p = calloc(header_chunk, sizeof(char));//malloc(sizeof(char) * header_chunk);

                status = RECEIVING_BODY;
            }
        } else if (status == RECEIVING_BODY) {
            append_bits((unsigned char) ((frequency - BASE_FREQ) / LINEAR_INTERVAL));
        }
    }
}

void append_bits(unsigned char bits) {

    printf("%i\n", -200);


    unsigned char* cur_byte = &((*decoded_bytes_p)[appended_bits_count / 8]);

    *cur_byte += bits << (8 - appended_bits_count % 8 - BITS_PER_TONE);

    appended_bits_count += BITS_PER_TONE;

    if (appended_bits_count / 8 >= *num_of_tones_for_data) {
        status = TRANSFER_COMPLETE;
    }

}

char* process_colors(char byte) {
    static char a[4];
    for (int i = 0; i < 4; ++i)
    {
        a[i] = (byte >> (6 - 2 * i)) & 3;
    }
    return a;
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
    if (compare_freq(freq, GUARD_FREQUENCY_TEXT)) return GUARD_FREQUENCY_TEXT;
    if (compare_freq(freq, GUARD_FREQUENCY_TEXT_B)) return GUARD_FREQUENCY_TEXT_B;
    if (compare_freq(freq, REPEAT_SEPARATOR_FREQUENCY)) return REPEAT_SEPARATOR_FREQUENCY;
    return 0;
}
