#include "hal.h"
#include "pwm_user.h"
#include "utils.h"

/******************************/
/*        Variables           */
/******************************/

PWMConfig pwm_config_tim1 = {
    100000, // period of about 10ms, set a smaller value to increase it
    1024,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
    },
    0,
    0
};

THD_WORKING_AREA(wa_led, 128);
/******************************/
/*         Functions          */
/******************************/
void pwm_set_pins(void) {
    palSetPadMode(GPIOE, GPIOE_9, PAL_MODE_ALTERNATE(1));
}

THD_FUNCTION(living_led, arg) {
    UNUSED(arg);
    palSetPadMode(GPIOF, 7, PAL_MODE_OUTPUT_PUSHPULL);
    while (TRUE) {
        palTogglePad(GPIOF, 7);
        chThdSleepMilliseconds(100);
    }
}
