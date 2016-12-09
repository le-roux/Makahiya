#ifndef PWM_USER_H
#define PWM_USER_H

#include "hal.h"
#include "stm32_tim.h"

extern THD_WORKING_AREA(wa_led, 128);

void pwm_set_pins(void);
THD_FUNCTION(living_led, arg);

#endif // PWM_USER_H
