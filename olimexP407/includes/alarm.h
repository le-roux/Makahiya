#ifndef ALARM_H
#define ALARM_H

#include "hal.h"

/**
 * @brief Maximum number of commands that can be executed on alarm_clock expiration.
 */
#define MAX_COMMANDS 16

/**
* @brief Tells whether the alarm has been started or not.
*/
extern int alarm_activated;

/**
 * @brief Mutex to protect @p alarm_activated.
 */
extern mutex_t lock;

/**
 * @brief Initialize all the elements required by the alarm driver.
 * WARNING: Never use the alarm driver before having called this function.
 */
void alarm_init(void);

/**
 * @brief Set the alarm_clock duration and the commands to run at expiration.
 * @param timeout Duration (in seconds) of the alarm.
 * @param commands_list
 */
void set_alarm(int timeout, char* commands_list);

#endif // ALARM_H
