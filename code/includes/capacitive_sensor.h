#ifndef CAPACITIVE_SENSOR_H
#define CAPACITIVE_SENSOR_H

#include <stdint.h>

#define BUFFER_SIZE 10
#define MARGIN 0.1

void init(void);
void update_default_value(void);
uint8_t touch_detected();
uint8_t add_value(float value);
float get_next_value(void);

#endif // CAPACITIVE_SENSOR_H
