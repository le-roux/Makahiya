#ifndef CAPACITIVE_SENSOR_H
#define CAPACITIVE_SENSOR_H

#include <stdint.h>

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
 * It first tries to detect a slide. To do this, it computes the linear
 * regression of the REGRESSION_SIZE last values stored in the buffer.
 * If the slope is sharp enough, it then checks that the last values is far
 * enough from the default_value (distance greater than SLIDE_MARGIN). This
 * This checks allows the algorithm to ignore noise.
 * If all these conditions are met, the status first goes to POTENTIAL_SLIDE.
 * If at the next call, a slide is still detected, the status finally goes to
 * IN_SLIDE and the proper value is returned. Otherwise, the status
 * immediately returns to DEFAULT_STATE.
 *
 * If no slide is detected, the algorithm then try to detect a touch.
 * This is basically a threshold overtaking check, performed on the average of
 * the values stored in the buffer.
 * If a touch is detected, the status goes to IN_TOUCH and the proper value is
 * returned.
 *
 * If nothing is detected, the status goes to DEFAULT_STATE and the proper value
 * is returned.
 *
 * @param sensor_id The index of the sensor to analyze.
 * @param channel_id Value in interval [0, 3] identifying the channel to analyze.
 * @return
 *  - 2 if a slide beginning is detected.
 *  - 1 if a touch is detected.
 *  - -1 when leaving the IN_TOUCH state
 *  - 0 otherwise (nothing detected).
 */
int detect_action(int sensor_id, int channel_id);

#endif // CAPACITIVE_SENSOR_H
