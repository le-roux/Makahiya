#include "capacitive_sensor.h"

uint8_t read_index, write_index, counter, turn, ret, in_touch;

uint32_t buffer[BUFFER_SIZE];

/** @brief Returns 1 if a touch is being detected, 0 otherwise.
 *
 * @return
 *  - 1 if a touch is being detected
 *  - 0 otherwise
 */
static uint8_t touch_detected(void);

void init(void) {
    read_index = 0;
    write_index = 0;
    average = 0;
    default_value = 0;
    in_touch = 0;
}

void update_default_value(void) {
    read_index = 0;
    write_index = 0;
    default_value = 0;
    for (counter = 0; counter < BUFFER_SIZE; counter++)
        default_value += buffer[counter];
    average = default_value;
}

static uint8_t touch_detected() {
    if (average > default_value + MARGIN || average < default_value - MARGIN)
        return 1;
    else
        return 0;
}

void add_value(uint32_t value) {
    average -= buffer[write_index];
    average += value;
    buffer[write_index] = value;
    write_index++;
    if (write_index >= BUFFER_SIZE) {
        turn++;
        write_index = 0;
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

uint32_t read_value(int index) {
    if (turn != 0 || index < write_index)
        return buffer[index];
    else
        return -1;
}

int detect_action(void) {
    // Detect slide

    // Detect touch
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
