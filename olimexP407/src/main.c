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
    palSetPadMode(GPIOF, 6, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPad(GPIOF, 6);
    sound_set_pins();
    chThdCreateStatic(wa_audio, sizeof(wa_audio), NORMALPRIO + 1, audio_playback, NULL);

    // Living led thread
    chThdCreateStatic(wa_led, sizeof(wa_led), NORMALPRIO - 1, living_led, NULL);

    // Receive data
    chThdCreateStatic(wa_audio_in, sizeof(wa_audio_in), NORMALPRIO + 1, audio_in, NULL);

    // Init the SerialUSB
    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);

    usbDisconnectBus(serusbcfg.usbp);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    // Loop forever.
    while (true) {
        chThdSleepMilliseconds(1000);
    }

    return 0;
}
