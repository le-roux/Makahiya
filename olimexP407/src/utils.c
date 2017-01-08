#include "utils.h"

void int_to_char(char* out, int value) {
    int index = 0, base = 10000;
    int end_set = 0;
    if (value > 99999) {
        out[0] = '\0';
        return;
    }

    for (int i = 0; i < 5; i++)
        out[i] = '0';

    for (int i = 5; i > 0; i--) {
        if (value >= base) {
            out[index] = (value / base) + '0';
            value -= (value / base) * base;
            index++;
            if (!end_set) {
                out[i] = '\0';
                end_set = 1;
            }
        }
        base /= 10;
    }
}
