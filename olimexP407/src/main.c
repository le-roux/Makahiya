// ChibiOS includes
#include "ch.h"
#include "hal.h"

// Includes for user code
#include "pwm_user.h"
#include "sound.h"

// Includes for the serial connection part
#include "chprintf.h"
#include "usbcfg.h"
#include "shell_user.h"

/*
 * Entry point
 */
int main(void) {

    // Init functions
    halInit();
    chSysInit();

    // Init the PWM
    pwm_set_pins();
    pwmInit();
    pwmStart(&PWMD1, &pwm_config_tim1);

    // Play sound
    sound_set_pins();
    chThdCreateStatic(wa_audio, sizeof(wa_audio), NORMALPRIO + 1, audio_playback, NULL);

    //sound_init();
    //sound_440();
    i2sStart(&I2SD3, &i2s3_cfg);
    i2sStartExchange(&I2SD3);


    // Init the SerialUSB
    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);

    usbDisconnectBus(serusbcfg.usbp);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    // Init the shell
    shellInit();

    // Loop forever taking care of the shell.
    while (true) {
        if (!shelltp && (SDU1.config->usbp->state == USB_ACTIVE))
            shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
        else if (chThdTerminatedX(shelltp)) {
            chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
            shelltp = NULL;           /* Triggers spawning of a new shell.        */
        }
        chThdSleepMilliseconds(1000);
    }

    return 0;
}
