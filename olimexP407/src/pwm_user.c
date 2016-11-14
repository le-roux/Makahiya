#include "hal.h"
#include "pwm_user.h"
#include "stm32_tim.h"

#if PWM_USE_TIM_10
PWMDriver PWMD10;

PWMConfig pwm_config_tim10 = {
    200000,
    1024,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
    },
    0,
    0
};
#endif

#if PWM_USE_TIM_11
PWMDriver PWMD11;

PWMConfig pwm_config_tim11 = {
    200000,
    1024,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
    },
    0,
    0
};
#endif

#if PWM_USE_TIM_13
PWMDriver PWMD13;

PWMConfig pwm_config_tim13 = {
    200000,
    1024,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    },
    0,
    0
};
#endif

#if PWM_USE_TIM_14
PWMDriver PWMD14;

PWMConfig pwm_config_tim14 = {
    200000,
    1024,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
    },
    0,
    0
};
#endif

PWMConfig pwm_config_tim3 = {
    1000000,
    1024,
    NULL,
    {
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL}
    },
    0,
    0
};

PWMConfig pwm_config_tim1 = {
    1000000,
    1024,
    NULL,
    {
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL}
    },
    0,
    0
};

PWMConfig pwm_config_tim4 = {
    1000000,
    1024,
    NULL,
    {
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL}
    },
    0,
    0
};

void pwm_init(void) {
    #if PWM_USE_TIM_10
    pwmObjectInit(&PWMD10);
    PWMD10.channels = STM32_TIM10_CHANNELS;
    PWMD10.tim = STM32_TIM10;
    #endif

    #if PWM_USE_TIM_11
    pwmObjectInit(&PWMD11);
    PWMD11.channels = STM32_TIM11_CHANNELS;
    PWMD11.tim = STM32_TIM11;
    #endif

    #if PWM_USE_TIM_13
    pwmObjectInit(&PWMD13);
    PWMD13.channels = STM32_TIM13_CHANNELS;
    PWMD13.tim = STM32_TIM13;
    #endif

    #if PWM_USE_TIM_14
    pwmObjectInit(&PWMD14);
    PWMD14.channels = STM32_TIM14_CHANNELS;
    PWMD14.tim = STM32_TIM14;
    #endif

}

void pwm_start(PWMDriver* driver, const PWMConfig* config) {
    /* Code from pwm.c */
    osalDbgCheck((driver != NULL) && (config != NULL));

    osalSysLock();
    osalDbgAssert((driver->state == PWM_STOP) || (driver->state == PWM_READY),
                  "invalid state");
    driver->config = config;
    driver->period = config->period;
    /* End */

    if (driver->state == PWM_STOP) {
        #if PWM_USE_TIM_10
        if (&PWMD10 == driver) {
            rccEnableTIM10(FALSE);
            rccResetTIM10();
        }
        #endif

        #if PWM_USE_TIM_11
        if (&PWMD11 == driver) {
            rccEnableTIM11(FALSE);
            rccResetTIM11();
        }
        #endif

        #if PWM_USE_TIM_13
        if (&PWMD13 == driver) {
            rccEnableTIM13(FALSE);
            rccResetTIM13();
        }
        #endif

        #if PWM_USE_TIM_14
        if (&PWMD14 == driver) {
            rccEnableTIM14(FALSE);
            rccResetTIM14();
        }
        #endif
        driver->clock = STM32_TIMCLK1;

        pwm_lld_start(driver);
    }

    /* Code from pwm.c */
    driver->enabled = 0;
    driver->state = PWM_READY;
    osalSysUnlock();
    /* End */
}

void pwm_stop(PWMDriver* driver) {

    /* Code from pwm.c */
    osalDbgCheck(driver != NULL);

    osalSysLock();
    osalDbgAssert((driver->state == PWM_STOP) || (driver->state == PWM_READY),
                    "invalid state");
    pwm_lld_stop(driver);
    /* End */
    #if PWM_USE_TIM_10
    if (&PWMD10 == driver)
        rccDisableTIM10(FALSE);
    #endif
    #if PWM_USE_TIM_11
    if (&PWMD11 == driver)
        rccDisableTIM11(FALSE);
    #endif
    #if PWM_USE_TIM_13
    if (&PWMD13 == driver)
        rccDisableTIM13(FALSE);
    #endif
    #if PWM_USE_TIM_14
    if (&PWMD14 == driver)
        rccDisableTIM14(FALSE);
    #endif
    /* Code from pwm.c */
    driver->enabled = 0;
    driver->state   = PWM_STOP;
    osalSysUnlock();
    /* End */
}

PWMDriver* PWM_ID(int id) {
    switch(id) {
        case(0): return PWMD(10);
        case(1): return PWMD(11);
        case(2): return PWMD(13);
        case(3): return PWMD(14);
        default:  return (PWMDriver*)-1;
    }
}
