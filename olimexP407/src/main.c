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
#include "utils.h"

#include "wifi.h"
#include "websocket.h"

/*
 * Entry point
 */
int main(void) {

    // Init functions
    halInit();
    chSysInit();

    // Init the SerialUSB
    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);

    usbDisconnectBus(serusbcfg.usbp);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    // Init the serial
    serial_set_pin();
    sdStart(wifi_SD, &serial_cfg);

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
    chThdCreateStatic(wa_audio_in, sizeof(wa_audio_in), NORMALPRIO + 2, wifi_audio_in, NULL);
    //chThdCreateStatic(wa_audio_in, sizeof(wa_audio_in), NORMALPRIO + 2, flash_audio_in, NULL);

    // Websocket thread
    //chThdCreateStatic(wa_websocket, sizeof(wa_websocket), NORMALPRIO + 1, websocket, "42");
    chThdSleepMilliseconds(1000);
    DEBUG("read music");

    // Init the shell
    shellInit();


    // Init the shell
    shellInit();

    // Init the serial
    sdInit();
    serial_set_pin();

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
