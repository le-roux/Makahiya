// ChibiOS includes
#include "ch.h"
#include "hal.h"

#include "pwm_user.h"
// Includes for the serial connection part
#include "chprintf.h"
#include "usbcfg.h"
#include "shell_user.h"
#include "i2c_user.h"
#include "fdc2214.h"
#include "ext_user.h"

/*
 * Entry point
 */
int main(void) {

    // Init functions
    halInit();
    chSysInit();

    // Living led thread
    chThdCreateStatic(wa_led, sizeof(wa_led), NORMALPRIO - 1, living_led, NULL);

    // Init the SerialUSB
    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);

    usbDisconnectBus(serusbcfg.usbp);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    // Init the shell
    shellInit();

    // Init the i2c
    i2c_set_pins();
    i2cStart(&I2CD2, &i2c2_cfg);
    i2cflags_t status;
    chThdSleepMilliseconds(1000);
    do {
        chThdSleepMilliseconds(100);
        status = init_sensor();
    } while (status != I2C_NO_ERROR);

    chThdCreateStatic(fdc_wa, sizeof(fdc_wa), NORMALPRIO + 2, fdc_int, NULL);
    extStart(&EXTD1, &ext_config);

    // Loop forever.
    while (true) {
        if (!shelltp & (SDU1.config->usbp->state == USB_ACTIVE))
            shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
        else if (chThdTerminatedX(shelltp)) {
            chThdRelease(shelltp);
            shelltp = NULL;
        }
        chThdSleepMilliseconds(1000);
    }

    return 0;
}
