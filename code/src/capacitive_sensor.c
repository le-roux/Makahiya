#include "capacitive_sensor.h"
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
 */
static int status[SENSORS_NB];

/**
 * Defines for the possible values of __status__.
 */
#define DEFAULT_STATE 0
#define IN_TOUCH 1
#define IN_SLIDE 2

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
    if (average[sensor_id] > default_value[sensor_id] + MARGIN || average[sensor_id] < default_value[sensor_id] - MARGIN)
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

uint32_t get_next_value(int sensor_id) {
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

/**
 * Blocking if there is no value to read when called.
 */
/*static*/ uint32_t read_value(int sensor_id, int index) {
    sem_wait(&read_sem[sensor_id]);
    if (turn[sensor_id] != 0 || index < write_index[sensor_id])
        return buffer[sensor_id * BUFFER_SIZE + index];
    else
        return -1;
}

int detect_action(int sensor_id) {
    // Detect slide

    // Detect touch
    if (touch_detected(sensor_id)) {
        if (status[sensor_id] == IN_TOUCH)
            return 0;
        else {
            status[sensor_id] = IN_TOUCH;
            return 1;
        }
    } else {
        status[sensor_id] = DEFAULT_STATE;
        return 0;
    }
}
