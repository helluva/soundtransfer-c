
#include <stdio.h>

#include "simplebinary.h"

int main() {
    printf("this number should be 400: %i\n", freq_8_from_input(0x1)[6]);
    printf("this number should be 800: %i\n", freq_8_from_input(0x1)[7]);

    printf("this number should be 300: %i\n", freq_4_from_input(0x0)[3]);
    printf("this number should be 500: %i\n", freq_4_from_input(0x1)[3]);
    printf("this number should be 700: %i\n", freq_4_from_input(0x2)[3]);
    printf("this number should be 900: %i\n", freq_4_from_input(0x3)[3]);
}
