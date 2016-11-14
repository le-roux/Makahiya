#ifndef PWM_USER_H
#define PWM_USER_H

#include "pwm_lld.h"

/*
 * Master switch to enable extra timers to be used for PWM
 */
#define PWM_USE_TIM_10 TRUE
#define PWM_USE_TIM_11 TRUE
#define PWM_USE_TIM_13 TRUE
#define PWM_USE_TIM_14 TRUE

#define PWMD(x) &PWMD ## x

#if PWM_USE_TIM_10
extern PWMDriver PWMD10;
extern PWMConfig pwm_config_tim10;
#endif

#if PWM_USE_TIM_11
extern PWMDriver PWMD11;
extern PWMConfig pwm_config_tim11;
#endif

#if PWM_USE_TIM_13
extern PWMDriver PWMD13;
extern PWMConfig pwm_config_tim13;
#endif

#if PWM_USE_TIM_14
extern PWMDriver PWMD14;
extern PWMConfig pwm_config_tim14;
#endif

extern PWMConfig pwm_config_tim3;
extern PWMConfig pwm_config_tim1;
extern PWMConfig pwm_config_tim4;
void pwm_init(void);

void pwm_start(PWMDriver* driver, const PWMConfig* config);

void pwm_stop(PWMDriver* driver);

PWMDriver* PWM_ID(int id);

#endif // PWM_USER_H
