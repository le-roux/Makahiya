#ifndef ALARM_H
#define ALARM_H

#include "hal.h"

/**
 * @brief Initialize all the elements required by the alarm driver.
 * WARNING: Never use the alarm driver before having called this function.
 */
void alarmInit(void);

/**
 * @brief Set the alarm_clock duration and the commands to run at expiration.
 * @param timeout Duration (in seconds) of the alarm.
 * @param commands_list
 *      command_id:
 *          - 1: start music
 *          - 60 + i: motor i
 *          - 70: sleep
 *          - for LEDs, see pwmdriver.h
 */
void set_alarm(int timeout, char* commands_list, char** save_ptr);

#endif // ALARM_H
