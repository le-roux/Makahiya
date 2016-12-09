#include "hal.h"
#include "pwm_user.h"
#include "utils.h"

/******************************/
/*        Variables           */
/******************************/
THD_WORKING_AREA(wa_led, 128);

/******************************/
/*         Functions          */
/******************************/

THD_FUNCTION(living_led, arg) {
    UNUSED(arg);
    palSetPadMode(GPIOC, 13, PAL_MODE_OUTPUT_PUSHPULL);
    while (TRUE) {
        palTogglePad(GPIOC, 13);
        chThdSleepMilliseconds(100);
    }
}
