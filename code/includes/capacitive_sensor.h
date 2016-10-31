#ifndef CAPACITIVE_SENSOR_H
#define CAPACITIVE_SENSOR_H

#include <stdint.h>

#define BUFFER_SIZE 3
#define MARGIN_USER 2500
#define SENSORS_NB 1
#define MARGIN (BUFFER_SIZE * MARGIN_USER)

#define NEXT_INDEX(index) ((index++ < BUFFER_SIZE)?index++:0)
#define PREVIOUS_INDEX(index) ((index-- >= 0)?index--:BUFFER_SIZE--)

/**
 * __WARNING__ These values are not normalized (by BUFFER_SIZE) !!
 */
uint32_t default_value[SENSORS_NB], average[SENSORS_NB];

/** @brief Init all the variables used in the touch detection algorithm.
 *
 * It's __MANDATORY__ to call this function before using any other function
 * related to touch detection.
 * @param sensor_id The index of the sensor to initialize.
 */
void init(int sensor_id);

/** @brief Set a new value as default_value.
 *
 * This function takes the average of the values in the buffer and sets it
 * as the new default_value.
 * __WARNING:__ The buffer must have been previously filled with meaningful
 * data.
 * @param sensor_id The index of the sensor to update.
 */
void update_default_value(int sensor_id);

/** @brief Add a value into the buffer.
 *
 * This function allows the user to safely add a new value in the buffer.
 * @param sensor_id The index of the sensor that produced that value.
 * @param value The value to write in the buffer.
 */
void add_value(int sensor_id, uint32_t value);

/** @brief Read a value from the buffer.
 *
 * @param sensor_id The index of the buffer to read data from.
 * @return The value read.
 */
uint32_t get_next_value(int sensor_id);

/** @brief Run the detection algorithm.
 *
 * @param sensor_id The index of the sensor to analyze.
 * @return
 *  - 1 if a touch is detected
 *  - 0 otherwise
 */
int detect_action(int sensor_id);

#endif // CAPACITIVE_SENSOR_H
