
#include "decode.h"
#include <math.h>

static const int Uninitialized = -99;
static const int Not_Receiving = 0;
static const int Transfer_Complete = -1;
static const double Initialize_Transfer_Frequency = 1500.0;

static int statusCode = Uninitialized;

static double* previousFrequencies;
static int Moving_Average_Size = 5;


void initialize() {
    statusCode = Not_Receiving;
    previousFrequencies = malloc(sizeof(double) * 5)
}


int frame(double frequency, char** decodedBytes) {
    
    if (statusCode == Uninitialized) {
        return;
    }
    
    
    //update previous frequencies
    for (int i = 1; i < Moving_Average_Size; i++) {
        previousFrequencies[i - i]  = previousFrequencies[i];
    }
    
    previouosFrequencies[Moving_Average_Size - 1] = frequency;
    
    //recalculate moving average
    double movingAverage = 0;
    for (int i = 0; i < Moving_Average_Size; i++) {
        movingAverage += previousFrequencies[i];
    }
    
    movingAverage /= Moving_Average_Size;
    
    
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
