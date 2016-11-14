#ifndef STATUS_LED_H
#define STATUS_LED_H

#include "hal.h"

#define LED_NB 4
#define STAT(n) GPIOF_STAT ## n

extern int brightness_1, brightness_2;


int LED_ID(int id);

void update_stat_led(void);
void update_stat_ledI(void);

#endif // STATUS_LED_H
