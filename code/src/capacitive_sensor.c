#include "capacitive_sensor.h"
#include "utils.h"
#include <semaphore.h>

/**
 * Array of the read indexes for the buffer(producer-consumer model).
 */
static int read_index[SENSORS_NB];

/**
 * Array of the write indexes for the buffer (producer-consumer model).
 */
static int write_index[SENSORS_NB];

/**
 * Array of status descriptor for the sensors.
 * Possible values are:
 *  - 0 : default state, nothing detected.
 *  - 1 : touch is being detected, don't report new touch.
 *  - 2 : slide is being detected.
 *  - 3 : potential slide detected, waiting for confirmation.
 *  - 4 : inactive (return from slide).
 */
static int status[SENSORS_NB];

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
static uint32_t buffer[BUFFER_SIZE * SENSORS_NB];

/**
 * Semaphores that control the reading from the buffer (one for each sensor).
 */
static sem_t read_sem[SENSORS_NB];

/**
 * Indicates that data have been lost because the input thread writes faster
 * that the output thread can read.
 */
//static int overflow[SENSORS_NB];

static int turn[SENSORS_NB];

/** @brief Returns 1 if a touch is being detected, 0 otherwise.
 *
 * @return
 *  - 1 if a touch is being detected
 *  - 0 otherwise
 */
static uint8_t touch_detected(int sensor_id);

void init(int sensor_id) {
    read_index[sensor_id] = 0;
    write_index[sensor_id] = 0;
    average[sensor_id] = 0;
    default_value[sensor_id] = 0;
    status[sensor_id] = DEFAULT_STATE;
    turn[sensor_id] = 0;
    sem_init(&read_sem[sensor_id], 0, 0);
}

void update_default_value(int sensor_id) {
    read_index[sensor_id] = 0;
    write_index[sensor_id] = 0;
    default_value[sensor_id] = 0;
    turn[sensor_id] = 0;
    int offset = sensor_id * BUFFER_SIZE;
    for (int i = 0; i < BUFFER_SIZE; i++)
        default_value[sensor_id] += buffer[offset + i];
    average[sensor_id] = default_value[sensor_id];
}

static uint8_t touch_detected(int sensor_id) {
    if (average[sensor_id] > default_value[sensor_id] + MARGIN ||
         average[sensor_id] < default_value[sensor_id] - MARGIN)
        return 1;
    else
        return 0;
}

void add_value(int sensor_id, uint32_t value) {
    int offset = sensor_id * BUFFER_SIZE;
    average[sensor_id] -= buffer[offset + write_index[sensor_id]];
    average[sensor_id] += value;
    buffer[offset + write_index[sensor_id]] = value;
    write_index[sensor_id]++;
    if (write_index[sensor_id] >= BUFFER_SIZE) {
        turn[sensor_id]++;
        write_index[sensor_id] = 0;
    }
    sem_post(&read_sem[sensor_id]);
}

/**
 * Blocking if there is no value to read when called.
 */
uint32_t get_next_value(int sensor_id) {
    sem_wait(&read_sem[sensor_id]);
    if (turn[sensor_id] != 0 || read_index[sensor_id] < write_index[sensor_id]) {
        uint32_t ret = buffer[sensor_id * BUFFER_SIZE + read_index[sensor_id]];
        read_index[sensor_id]++;
        if (read_index[sensor_id] >= BUFFER_SIZE) {
            turn[sensor_id]--;
            read_index[sensor_id] = 0;
        }
        return ret;
    } else
        return -1;
}

/*static*/ uint32_t read_value(int sensor_id, int index) {
    if (turn[sensor_id] != 0 || index < write_index[sensor_id])
        return buffer[sensor_id * BUFFER_SIZE + index];
    else
        return -1;
}

#include <stdio.h>
int linear_regression(int sensor_id) {
    double average_x = ((double)(REGRESSION_SIZE * (REGRESSION_SIZE - 1)))/(2 * REGRESSION_SIZE);
    double average_y = 0;
    double var_x = 0, cov_xy = 0;
    int offset = sensor_id * BUFFER_SIZE;
    int index = PREVIOUS_INDEX(write_index[sensor_id]);
    for (int i = REGRESSION_SIZE - 1; i >= 0; i--) {
        average_y += buffer[offset + index];
        var_x += i*i;
        cov_xy += i * buffer[offset + index];
        index = PREVIOUS_INDEX(index);
    }
    average_y /= REGRESSION_SIZE;
    var_x /= REGRESSION_SIZE;
    var_x -= average_x*average_x;
    cov_xy /= REGRESSION_SIZE;
    cov_xy -= average_x * average_y;
    return (int)(cov_xy/var_x);
}

int detect_action(int sensor_id) {
    // Detect slide
    if (status[sensor_id] != IN_TOUCH) {
        int coeff = ABS(linear_regression(sensor_id));
        if (coeff > 200 && coeff < 300) {
            if (status[sensor_id] != IN_SLIDE &&
                current_distance(sensor_id) > SLIDE_MARGIN) {
                if (status[sensor_id] == POTENTIAL_SLIDE) {
                    status[sensor_id] = IN_SLIDE;
                    return 2;
                } else {
                    status[sensor_id] = POTENTIAL_SLIDE;
                    return 0;
                }
            } else
                return 0;
        }
    }

    // Leave slide state
    if ((status[sensor_id] == IN_SLIDE && average[sensor_id] < 100) ||
        status[sensor_id] == POTENTIAL_SLIDE)
        status[sensor_id] = DEFAULT_STATE;

    // Detect touch
    if (status[sensor_id] != IN_SLIDE && touch_detected(sensor_id)) {
        if (status[sensor_id] == IN_TOUCH)
            return 0;
        else {
            status[sensor_id] = IN_TOUCH;
            return 1;
        }
    }

    // Leave touch state
    if (status[sensor_id] == IN_TOUCH && !touch_detected(sensor_id)) {
        status[sensor_id] = DEFAULT_STATE;
    }

    // Default return value
    return 0;
}

int current_distance(int sensor_id) {
    return (default_value[sensor_id]/BUFFER_SIZE - buffer[sensor_id * BUFFER_SIZE + PREVIOUS_INDEX(write_index[sensor_id])]);
}
