#ifndef CAPACITIVE_SENSOR_H
#define CAPACITIVE_SENSOR_H

#include <stdint.h>

#define BUFFER_SIZE 3
#define MARGIN_USER 2500
#define MARGIN (BUFFER_SIZE * MARGIN_USER)

#define NEXT_INDEX(index) ((index++ < BUFFER_SIZE)?index++:0)
#define PREVIOUS_INDEX(index) ((index-- >= 0)?index--:BUFFER_SIZE--)

/**
 * __WARNING__ These values are not normalized (by BUFFER_SIZE) !!
 */
uint32_t default_value, average;

/** @brief Status indicator.
 *
 * Indicates if we're in a touch (in which case a touch mustn't be reported
 * before we leave this state) or not.
 */
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

/** @brief Add a value into the buffer.
 *
 * This function allows the user to safely add a new value in the buffer.
 */
void add_value(uint32_t value);

/** @brief Read a value from the buffer.
 *
 *
 * @return The value read.
 */
uint32_t get_next_value(void);

/** @brief Run the detection algorithm.
 *
 * @return
 *  - 1 if a touch is detected
 *  - 0 otherwise
 */
int detect_action(void);

#endif // CAPACITIVE_SENSOR_H
