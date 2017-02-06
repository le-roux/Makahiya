#ifndef CAPACITIVE_SENSOR_H
#define CAPACITIVE_SENSOR_H

#include <stdint.h>

/**
 * Enum describing the possible return values for @p detect_action
 */
typedef enum {NO_ACTION, NEW_TOUCH, LEAVE_TOUCH} action_t;

/**
 * Enum describing the possible state of the algorithm.
 */
typedef enum {DEFAULT_STATE, IN_TOUCH} status_t;

/**
 * Size of the buffer used to store the values from the sensor.
 */
#define BUFFER_SIZE 1

#define INT_DER_VERSION 0
#define DERIVATIVE_THRESHOLD 1000
#define INTEGRAL_THRESHOLD 10000

/**
 * Number of sensors used.
 */
#define SENSORS_NB 2

/**
 * Maximum number of channels per sensor.
 */
#define MAX_CHANNELS_NB 4

/**
 * Gives the index in the buffer following the one given in parameter.
 */
#define NEXT_INDEX(index) ((index+1 < BUFFER_SIZE)?index+1:0)

/**
 * Gives the index in the buffer preceding the one given in parameter.
 */
#define PREVIOUS_INDEX(index) ((index-1 >= 0)?index-1:BUFFER_SIZE-1)

/** @brief Init all the variables used in the touch detection algorithm.
 *
 * It's __MANDATORY__ to call this function before using any other function
 * related to touch detection.
 *
 * @param sensor_id The index of the sensor to initialize (value in interval [0, 1]).
 * @paramm channel_id Value in interval [0, 3] identifying the channel.
 */
void init_touch_detection(int sensor_id, int channel_id);

/** @brief Add a value into the buffer.
 *
 * This function allows the user to safely add a new value in the buffer.
 * @param sensor_id The index of the sensor that produced that value.
 * @param channel_id Value in interval [0,3] identifying the channel that
 *                      produced this data.
 * @param value The value to write in the buffer.
 */
void add_value(int sensor_id, int channel_id, uint32_t value);

/** @brief Run the detection algorithm.
 *
 * The algorithm tries to detect a touch.
 * It simply compares the last value read to a defined threshold. If the threshold
 * is overtaken, it goes to the IN_TOUCH state and the proper value is returned.
 *
 * When the value then becomes smaller than the threshold, the algorithm goes back to
 * the DEFAULT_STATE and the proper value is returned.
 *
 * @param sensor_id The index of the sensor to analyze.
 * @param channel_id Value in interval [0, 3] identifying the channel to analyze.
 * @return
 *  - NEW_TOUCH if a touch is detected.
 *  - LEAVE_TOUCH if the IN_TOUCH state is left.
 *  - NO_ACTION otherwise (nothing detected).
 */
action_t detect_action(int sensor_id, int channel_id);

#endif // CAPACITIVE_SENSOR_H
