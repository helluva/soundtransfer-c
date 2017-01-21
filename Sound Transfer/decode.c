
#include "globals.h"
#include "decode.h"
#include <math.h>
#include <stdlib.h>

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


int frequenciesMatch(double frequency, double target) {
    double threshold = 25.0;
    double difference = abs(frequency - target);
    return difference < threshold;
}


int frame(double frequency, char** decodedBytes) {

    if (statusCode == Uninitialized) {
        return statusCode;
    }


    //build the 9 previous frequencies
    if (previousFrequenciesCount < Samples_Per_Encoded_Frequency) {
        previousFrequencies[previousFrequenciesCount] = frequency;
        previousFrequenciesCount += 1;
    }

    if (previousFrequenciesCount == Samples_Per_Encoded_Frequency) {
        
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
        
        return mostCommonFrequency;
        
    }

    return statusCode;
}
