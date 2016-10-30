#ifndef CAPACITIVE_SENSOR_H
#define CAPACITIVE_SENSOR_H

#include <stdint.h>

#define BUFFER_SIZE 3
#define MARGIN 2500

uint32_t default_value, average;
extern uint8_t in_touch;

void init(void);
void update_default_value(void);
uint8_t touch_detected();
uint8_t add_value(uint32_t value);
uint32_t get_next_value(void);

#endif // CAPACITIVE_SENSOR_H
