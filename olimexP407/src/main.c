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
#include "serial_user.h"

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

    palSetPadMode(GPIOF, 6, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPad(GPIOF, 6);

    // Living led thread
    chThdCreateStatic(wa_led, sizeof(wa_led), NORMALPRIO - 1, living_led, NULL);

    // Audio threads
    sound_set_pins();
    chThdCreateStatic(wa_audio, sizeof(wa_audio), NORMALPRIO + 1, audio_playback, NULL);
    chThdCreateStatic(wa_audio_in, sizeof(wa_audio_in), NORMALPRIO + 2, audio_in, NULL);

    // Init the SerialUSB
    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);

    usbDisconnectBus(serusbcfg.usbp);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    // Init the serial
    serial_set_pin();
    sdStart(wifi_SD, &serial_cfg);

    // Init the shell
    shellInit();


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
