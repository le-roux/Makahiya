#include "capacitive_sensor.h"
#include "hal.h"
#include "RTT_streams.h"
#include "chprintf.h"
#include "utils.h"
#include "pwmdriver.h"

/**
 * Array of the write indexes for the buffer.
 */
static volatile int write_index[SENSORS_NB][MAX_CHANNELS_NB];

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
static volatile float buffer[BUFFER_SIZE * SENSORS_NB * MAX_CHANNELS_NB];

/** @brief Returns 1 if a touch is being detected, 0 otherwise.
 *
 * @return
 *  - 1 if a touch is being detected
 *  - 0 otherwise
 */

void init_touch_detection(int sensor_id, int channel_id) {
	write_index[sensor_id][channel_id] = 0;
	status[sensor_id][channel_id] = DEFAULT_STATE;
#if INT_DER_VERSION
	integral[sensor_id][channel_id] = 0;
#endif // INT_DER_VERSION
}

#if !INT_DER_VERSION
static uint8_t touch_detected(int sensor_id, int channel_id) {

	int offset = (sensor_id * MAX_CHANNELS_NB + channel_id) * BUFFER_SIZE;
	int last_val = buffer[offset + PREVIOUS_INDEX(write_index[sensor_id][channel_id])];

	if (last_val < 3000000)
		return 1;
	else
		return 0;

}

#endif

void add_value(int sensor_id, int channel_id, uint32_t value) {
	/**
	 * Offset in the buffer of samples.
	 */
	int offset = (sensor_id * MAX_CHANNELS_NB + channel_id) * BUFFER_SIZE;

	// Actually write the fvalue in the buffer.
	buffer[offset + write_index[sensor_id][channel_id]] = value;

	// Update the write_index fvalue.
	write_index[sensor_id][channel_id]++;
	if (write_index[sensor_id][channel_id] >= BUFFER_SIZE)
		write_index[sensor_id][channel_id] = 0;
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
