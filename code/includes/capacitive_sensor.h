#ifndef CAPACITIVE_SENSOR_H
#define CAPACITIVE_SENSOR_H

#include <stdint.h>

/**
 * Size of the buffer used to store the values from the sensor.
 */
#define BUFFER_SIZE 10

/**
 * Margin to detect a touch (per value).
 */
#define MARGIN_USER 3100

/**
 * Margin used to detect the beginning of a slide.
 */
#define SLIDE_MARGIN 3000

/**
 * Number of sensors used.
 */
#define SENSORS_NB 1

/**
 * Margin used to detect a touch, due to the implementation.
 * __WARNING:__ Don't modify this value.
 */
#define MARGIN (BUFFER_SIZE * MARGIN_USER)

/**
 * Number of values used to perform the linear regression.
 * __WARNING:__ This value MUST be less than BUFFER_SIZE.
 */
#define REGRESSION_SIZE 7

/**
 * Gives the index in the buffer following the one given in parameter.
 */
#define NEXT_INDEX(index) ((index+1 < BUFFER_SIZE)?index+1:0)

/**
 * Gives the index in the buffer preceding the one given in parameter.
 */
#define PREVIOUS_INDEX(index) ((index-1 >= 0)?index-1:BUFFER_SIZE-1)

/**
 * __WARNING__ This value is not normalized (by BUFFER_SIZE) !!
 */
uint32_t average[SENSORS_NB];

extern uint32_t default_value[SENSORS_NB];

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


int linear_regression(int sensor_id);

int current_distance(int sensor_id);

#endif // CAPACITIVE_SENSOR_H
