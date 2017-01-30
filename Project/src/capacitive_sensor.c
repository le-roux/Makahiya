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
 *  - 0 : default state, nothing detected.
 *  - 1 : touch is being detected, don't report new touch.
 *  - 2 : slide is being detected.
 *  - 3 : potential slide detected, waiting for confirmation.
 *  - 4 : inactive (return from slide).
 */
static int status[SENSORS_NB][MAX_CHANNELS_NB];

/**
 * Defines for the possible values of __status__.
 */
#define DEFAULT_STATE 0
#define IN_TOUCH 1
#define IN_SLIDE 2
#define POTENTIAL_SLIDE 3
#define INACTIVE 4

/**
 * The buffer used to store data from the capacitive sensors.
 */
static volatile float buffer[BUFFER_SIZE * SENSORS_NB * MAX_CHANNELS_NB];

/**
 * Array storing for each channel the average of the last BUFFER_SIZE values.
 */
float average[SENSORS_NB][MAX_CHANNELS_NB];

/**
 * Array storing for each channel the default value.
 */
float default_value[SENSORS_NB][MAX_CHANNELS_NB];

static int DETECT_SLIDE = 0;

#if INT_DER_VERSION
/**
 * Array storing for each channel the integral value.
 */
static float integral[SENSORS_NB][MAX_CHANNELS_NB];

/**
 * Array storing for each channel the derivative value.
 */
static float derivative[SENSORS_NB][MAX_CHANNELS_NB];
#endif

/** @brief Returns 1 if a touch is being detected, 0 otherwise.
 *
 * @return
 *  - 1 if a touch is being detected
 *  - 0 otherwise
 */

void init_touch_detection(int sensor_id, int channel_id) {
	write_index[sensor_id][channel_id] = 0;
	average[sensor_id][channel_id] = 0;
	default_value[sensor_id][channel_id] = 0;
	status[sensor_id][channel_id] = DEFAULT_STATE;
#if INT_DER_VERSION
	integral[sensor_id][channel_id] = 0;
#endif // INT_DER_VERSION
}

void update_default_value(int sensor_id, int channel_id) {
	write_index[sensor_id][channel_id] = 0;
	default_value[sensor_id][channel_id] = 0;
	int offset = (sensor_id * MAX_CHANNELS_NB + channel_id) * BUFFER_SIZE;
	for (int i = 0; i < BUFFER_SIZE; i++)
		default_value[sensor_id][channel_id] += buffer[offset + i];
	average[sensor_id][channel_id] = default_value[sensor_id][channel_id];
}

#if !INT_DER_VERSION
static uint8_t touch_detected(int sensor_id, int channel_id) {
	/*int offset = (sensor_id * MAX_CHANNELS_NB + channel_id) * BUFFER_SIZE;
	float last_val = buffer[offset + PREVIOUS_INDEX(write_index[sensor_id][channel_id])] * BUFFER_SIZE;
	float mean = average[sensor_id][channel_id];
	if (sensor_id == 0 && channel_id == 3)
		DEBUG("%f %f\n", last_val, mean);
	if (last_val > mean) {
		if (last_val - mean > last_val / 5) 
			return 1;
		else
			return 0;
	} else {
		if (mean - last_val > mean / 5) 
			return 1;
		else 
			return 0;
	}*/
	
	int offset = (sensor_id * MAX_CHANNELS_NB + channel_id) * BUFFER_SIZE;
	int last_val = buffer[offset + PREVIOUS_INDEX(write_index[sensor_id][channel_id])];

	DEBUG("%d", last_val);

	if (last_val < 268435455)
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

	// Update the average.
	average[sensor_id][channel_id] -= buffer[offset + write_index[sensor_id][channel_id]];
	average[sensor_id][channel_id] += value;

	// Actually write the fvalue in the buffer.
	buffer[offset + write_index[sensor_id][channel_id]] = value;

	// Update the write_index fvalue.
	write_index[sensor_id][channel_id]++;
	if (write_index[sensor_id][channel_id] >= BUFFER_SIZE)
		write_index[sensor_id][channel_id] = 0;
}

reg_t linear_regression(int sensor_id, int channel_id) {
	double average_x = ((double)(REGRESSION_SIZE * (REGRESSION_SIZE - 1)))/(2 * REGRESSION_SIZE);
	uint64_t average_y = 0;
	double var_x = 0, cov_xy = 0;
	reg_t ret;
	int offset = (sensor_id * MAX_CHANNELS_NB + channel_id) * BUFFER_SIZE;
	int index = PREVIOUS_INDEX(write_index[sensor_id][channel_id]);
	for (int i = REGRESSION_SIZE - 1; i >= 0; i--) {
		average_y += buffer[offset + index];
		var_x += i*i;
		cov_xy += i * buffer[offset + index];
		index = PREVIOUS_INDEX(index);
	}
	average_y /= REGRESSION_SIZE;
	var_x /= REGRESSION_SIZE;
	var_x -= average_x * average_x;
	index = PREVIOUS_INDEX(write_index[sensor_id][channel_id]);
	ret.var_y = 0;
	for (int i = 0; i < REGRESSION_SIZE; i++) {
		ret.var_y += (buffer[offset + index] - average_y) * (buffer[offset + index] - average_y);
		index = PREVIOUS_INDEX(index);
	}
	ret.var_y /= REGRESSION_SIZE;
	cov_xy /= REGRESSION_SIZE;
	cov_xy -= average_x * average_y;
	ret.corr = (cov_xy * cov_xy) / (var_x * ret.var_y);
	ret.slope = (int)(cov_xy/var_x);
	return ret;
}

int detect_action(int sensor_id, int channel_id) {

	// Detect slide
	if (DETECT_SLIDE) {
		if (status[sensor_id][channel_id] != IN_TOUCH) {
			reg_t ret = linear_regression(sensor_id, channel_id);
			int coeff = ABS(ret.slope);
			if (coeff > 200 && coeff < 300) {
				if (status[sensor_id][channel_id] != IN_SLIDE &&
						current_distance(sensor_id, channel_id) > SLIDE_MARGIN && ret.corr > 0.75) {
					if (status[sensor_id][channel_id] == POTENTIAL_SLIDE) {
						status[sensor_id][channel_id] = IN_SLIDE;
						return 2;
					} else {
						status[sensor_id][channel_id] = POTENTIAL_SLIDE;
						return 0;
					}
				} else
					return 0;
			}
		}

		// Leave slide state
		if ((status[sensor_id][channel_id] == IN_SLIDE &&
					average[sensor_id][channel_id] > default_value[sensor_id][channel_id] - 500)
				|| status[sensor_id][channel_id] == POTENTIAL_SLIDE)
			status[sensor_id][channel_id] = DEFAULT_STATE;
		else if (status[sensor_id][channel_id] == IN_SLIDE)
			return 0;
	}

	// Detect touch
#if !INT_DER_VERSION
	
	if (status[sensor_id][channel_id] == 0 && touch_detected(sensor_id, channel_id)) {
		DEBUG("On");
		status[sensor_id][channel_id] = 1;
		return 1;
	} else if (status[sensor_id][channel_id] && touch_detected(sensor_id, channel_id) == 0) {
		DEBUG("Off");
		status[sensor_id][channel_id] = 0;
		return -1;
	}

	// Default return value
	return 0;
#else
	derivative[sensor_id][channel_id] = buffer[write_index[sensor_id][channel_id]] - buffer[PREVIOUS_INDEX(write_index[sensor_id][channel_id])];
	if (ABS(derivative[sensor_id][channel_id]) > DERIVATIVE_THRESHOLD)
		integral[sensor_id][channel_id] += derivative[sensor_id][channel_id];
	if (integral[sensor_id][channel_id] > INTEGRAL_THRESHOLD) {
		if (status[sensor_id][channel_id] != IN_TOUCH) {
			status[sensor_id][channel_id] = IN_TOUCH;
			return 1;
		} else
			return 0;
	} else {
		status[sensor_id][channel_id] = DEFAULT_STATE;
		integral[sensor_id][channel_id] *= 0.9;
		return 0;
	}
#endif
}

int32_t current_distance(int sensor_id, int channel_id) {
	uint32_t value = buffer[(sensor_id * MAX_CHANNELS_NB + channel_id) * BUFFER_SIZE
		+ PREVIOUS_INDEX(write_index[sensor_id][channel_id])];
	return (default_value[sensor_id][channel_id]/BUFFER_SIZE - value);
}
