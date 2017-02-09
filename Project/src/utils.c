#include "utils.h"

static int const MAX_BASE = 100000000;

void int_to_char(char* out, int value) {
    int index = 0;
    int base;
    if (value > ((MAX_BASE * 10) - 1)) {
        out[0] = '\0';
        return;
    } else if (value == 0) {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    base = 10;
    int char_nb = 1;
    while (value >= base) {
        char_nb++;
        base *= 10;
    }
    base /= 10;
    for (int i = char_nb; i > 0; i--) {
        out[index] = (value / base) + '0';
        index++;
        value -= (value / base) * base;
        base /= 10;
    }

    out[char_nb] = '\0';
}
