#ifndef CAPACITIVE_SENSOR_H
#define CAPACITIVE_SENSOR_H

#include <stdint.h>

#define BUFFER_SIZE 3
#define MARGIN 2500

uint32_t default_value, average;
extern uint8_t in_touch;

/** @brief Init all the variables used in the touch detection algorithm.
 *
 * It's __MANDATORY__ to call this function before using any other function
 * related to touch detection.
 */
void init(void);

/** @brief Set a new value as default_value.
 *
 * This function takes the average of the values in the buffer and sets it
 * as the new default_value.
 * __WARNING:__ The buffer must have been previously filled with meaningful
 * data.
 */
void update_default_value(void);

/** @brief Returns 1 if a touch is being detected, 0 otherwise.
 *
 * @return
 *  - 1 if a touch is being detected
 *  - 0 otherwise
 */
uint8_t touch_detected(void);

/** @brief Add a value into the buffer.
 *
 * This function allows the user to safely add a new value in the buffer.
 * It doesn't simply add the value but also run the touch detection algorithm.
 *
 * @param value: The value to write in the buffer.
 * @return
 *  - 1 if a touch is detected
 *  - 0 otherwise
 */
uint8_t add_value(uint32_t value);

/** @brief Read a value from the buffer.
 *
 *
 * @return The value read.
 */
uint32_t get_next_value(void);

#endif // CAPACITIVE_SENSOR_H
