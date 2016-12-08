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
#define MARGIN_USER 5000

#define INT_DER_VERSION 0
#define DERIVATIVE_THRESHOLD 1000
#define INTEGRAL_THRESHOLD 10000

/**
 * Margin used to detect the beginning of a slide.
 */
#define SLIDE_MARGIN 1500

/**
 * Number of sensors used.
 */
#define SENSORS_NB 1

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

typedef struct reg_t {
    int slope;
    double corr;
    uint64_t var_y;
} reg_t;

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
 * @return
 *  - 2 if a slide beginning is detected.
 *  - 1 if a touch is detected.
 *  - 0 otherwise (nothing detected).
 */
int detect_action(int sensor_id);

/** @brief Performs a linear regression.
 *
 * The regression is performed on the last REGRESSION_SIZE values of the buffer.
 * The least squares method is used.
 *
 * @param sensor_id The index of the sensor to analyze.
 * @return The slope of the linear regression (a if we have y = a*x+b).
 */
reg_t linear_regression(int sensor_id);

/** @brief Gives the diffence between the default value and the last written one.
 *
 * @param sensor_id The index of the sensor to analyze.
 * @ return The difference between the default value and the last written one.
 */
int32_t current_distance(int sensor_id);

#endif // CAPACITIVE_SENSOR_H
