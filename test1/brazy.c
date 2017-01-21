
#include <stdio.h>
#include <math.h>

#include "simplebinary.h"

int main() {

    const double R=8000; // sample rate (samples per second)
    const double C=261.625565; // frequency of middle-C (hertz)
    const double F=R/256; // bytebeat frequency of 1*t due to 8-bit truncation (hertz)
    const double V=127; // a volume constant

    int* arr = freq_4_from_input(0x73);
    for (int i = 0; i < 4; ++i) {
        for (int t = 0; t < R / 2; ++t) {
            unsigned int temp = ( sin(t * 2 * 3.1415926 / R * arr[i]) + 1 ) * V; // pure middle C sine wave
            // uint8_t temp = t/F*C; // middle C saw wave (bytebeat style)
            // uint8_t temp = (t*5&t>>7)|(t*3&t>>10); // viznut bytebeat composition
            printf("%i", temp);
        }
    }


}
