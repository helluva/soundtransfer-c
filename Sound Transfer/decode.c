
#include "globals.h"
#include "decode.h"
#include <math.h>

static const int Uninitialized = -99;
static const int Not_Receiving = 0;
static const int Transfer_Complete = -1;

static int statusCode = Uninitialized;


static const double Samples_Per_Second = 36.0;
static const double Encoded_Frequencies_Per_Second = 4.0;
static const double Samples_Per_Encoded_Frequency = 9;

static const double Initialize_Transfer_Frequency = 1500.0;


static double* previousFrequencies;
static int previousFrequenciesCount = 0;


void initialize() {
    statusCode = Not_Receiving;
    previousFrequencies = malloc(sizeof(double) * Samples_Per_Encoded_Frequency);
}


int frame(double frequency, char** decodedBytes) {

    if (statusCode == Uninitialized) {
        return;
    }


    //build the 9 previous frequencies
    if (previousFrequenciesCount < Samples_Per_Encoded_Frequency) {
        previousFrequencies[previousFrequenciesCount] = frequency;
        previousFrequenciesCount += 1
    }

    if (previousFrequenciesCount == Samples_Per_Encoded_Frequency) {



    }


    /* user moving average to determine next action */

    //if not receiving, listen for trigger
    if (statusCode == Not_Receiving) {
        if (frequenciesMatch(movingAverage, Initialize_Transfer_Frequency)) {
            //i dunno
        }
    }

    return statusCode;
}

int frequenciesMatch(double frequency, double target) {
    double threshold = 25.0;
    double difference = abs(frequency - target);
    return difference < threshold;
}
