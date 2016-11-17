#ifndef PWM_USER_H
#define PWM_USER_H

#include "hal.h"
#include "stm32_tim.h"

extern PWMConfig pwm_config_tim1;

void pwm_set_pins(void);

#endif // PWM_USER_H
