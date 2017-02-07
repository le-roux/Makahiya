#include "capacitive_sensor.h"
#include "hal.h"
#include "RTT_streams.h"
#include "chprintf.h"
#include "utils.h"
#include "pwmdriver.h"

/**
 * Enum describing the possible state of the algorithm.
 */
typedef enum {DEFAULT_STATE, IN_TOUCH} status_t;

/**
 * Array of status descriptor for the sensors.
 * Possible values are:
 *  - DEFAULT_STATE : default state, nothing detected.
 *  - IN_TOUCH : touch is being detected, don't report new touch.
 */
static status_t status[SENSORS_NB][MAX_CHANNELS_NB];

/**
 * The buffer used to store data from the capacitive sensors.
 */
static volatile float values[SENSORS_NB][MAX_CHANNELS_NB];

/** @brief Returns 1 if a touch is being detected, 0 otherwise.
 *
 * @return
 *  - 1 if a touch is being detected
 *  - 0 otherwise
 */
void init_touch_detection(int sensor_id, int channel_id) {
	status[sensor_id][channel_id] = DEFAULT_STATE;
}

// Threshold used to detect touches.
#define THRESHOLD 3000000

static uint8_t touch_detected(int sensor_id, int channel_id) {
	return values[sensor_id][channel_id] < THRESHOLD;
}

void add_value(int sensor_id, int channel_id, uint32_t value) {
	values[sensor_id][channel_id] = value;
}

action_t detect_action(int sensor_id, int channel_id) {

	// Detect touch

	if (status[sensor_id][channel_id] == DEFAULT_STATE && touch_detected(sensor_id, channel_id)) {
		status[sensor_id][channel_id] = IN_TOUCH;
		return NEW_TOUCH;
	} else if (status[sensor_id][channel_id] == IN_TOUCH && !touch_detected(sensor_id, channel_id)) {
		status[sensor_id][channel_id] = DEFAULT_STATE;
		return LEAVE_TOUCH;
	}

	// Default return value
	return NO_ACTION;
}
