#include "capacitive_sensor.h"

uint8_t read_index, write_index, counter, turn, ret, detect_touch, in_touch;

uint32_t buffer[BUFFER_SIZE];

void init(void) {
    read_index = 0;
    write_index = 0;
    average = 0;
    default_value = 0;
    detect_touch = 0;
    in_touch = 0;
}

void update_default_value(void) {
    read_index = 0;
    write_index = 0;
    default_value = 0;
    for (counter = 0; counter < BUFFER_SIZE; counter++)
        default_value += buffer[counter];
    default_value /= BUFFER_SIZE;
    average = default_value;
    detect_touch = 1;
}

uint8_t touch_detected() {
    if (average > default_value + MARGIN || average < default_value - MARGIN)
        return 1;
    else
        return 0;
}

uint8_t add_value(uint32_t value) {
    average -= buffer[write_index] / BUFFER_SIZE;
    average += value / BUFFER_SIZE;
    buffer[write_index] = value;
    write_index++;
    if (write_index >= BUFFER_SIZE) {
        turn++;
        write_index = 0;
    }
    if (touch_detected()) {
        if (in_touch)
            return 0;
        else {
            in_touch = 1;
            return 1;
        }
    } else {
        in_touch = 0;
        return 0;
    }
}

uint32_t get_next_value(void) {
    if (turn != 0 || read_index < write_index) {
        ret = buffer[read_index];
        read_index++;
        if (read_index >= BUFFER_SIZE) {
            turn--;
            read_index = 0;
        }
        return ret;
    } else
        return -1;
}
