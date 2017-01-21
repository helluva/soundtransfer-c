
#include "globals.h"
#include "decode.h"
#include <math.h>
#include <stdlib.h>

static const int UNINITIALIZED = -99;
static const int NOT_RECEIVING = -1;
static const int RECEIVING_HEADER = 1;
static const int TRANSFER_COMPLETE = 0;

static int statusCode = UNINITIALIZED;





static double* previousFrequencies;
static int previousFrequenciesCount = 0;


void init(int* length, char** decodedBytes) {
    statusCode = NOT_RECEIVING;
    previousFrequencies = malloc(sizeof(double) * SAMPLES_PER_ENCODED_FREQUENCY);
}


int frequenciesMatch(double frequency, double target) {
    double threshold = 25.0;
    double difference = abs(frequency - target);
    return difference < threshold;
}


int receive_frame(double frequency) {

    if (statusCode == UNINITIALIZED) {
        return statusCode;
    }


    //build the 9 previous frequencies
    if (previousFrequenciesCount < SAMPLES_PER_ENCODED_FREQUENCY) {
        previousFrequencies[previousFrequenciesCount] = frequency;
        previousFrequenciesCount += 1;
    }

    if (previousFrequenciesCount == SAMPLES_PER_ENCODED_FREQUENCY) {

        //build "dictionary" of valid frequencies
        int validFrequenciesCount = 16;
        int expectedFrequencies[validFrequenciesCount];
        int matchCount[validFrequenciesCount];

        int base = 1000; //replace with global constant
        int increment = 100; //replace with global constant

        for (int i = 0; i < validFrequenciesCount; i++) {
            matchCount[i] = 0;
            expectedFrequencies[i] = base + (increment * i);
        }

        //attribute previous frequencies to valid frequencies
        for (int i = 0; i < previousFrequenciesCount; i++) {
            int frequencyToAnalyze = previousFrequencies[i];

            for (int j = 0; j < validFrequenciesCount; j++){
                int possibleMatch = expectedFrequencies[j];
                if (frequenciesMatch(frequencyToAnalyze, possibleMatch)) {
                    matchCount[j] = matchCount[j] + 1;
                }
            }
        }

        //find most common of the expected frequencies
        int mostCommonFrequency = -1;
        int mostCommonFrequencyCount = 0;

        for (int j = 0; j < validFrequenciesCount; j++) {
            if (matchCount[j] > mostCommonFrequencyCount) {
                mostCommonFrequency = expectedFrequencies[j];
                mostCommonFrequencyCount = matchCount[j];
            }
        }


        //reset previous frequencies
        previousFrequenciesCount = 0;
        for (int i = 0; i < SAMPLES_PER_ENCODED_FREQUENCY; i++) {
            previousFrequencies[i] = 0;
        }

        if (mostCommonFrequency != -1 && mostCommonFrequencyCount > 4) {
            return mostCommonFrequency;
        }
    }

    return statusCode;
}
