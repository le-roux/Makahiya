// ChibiOS includes
#include "ch.h"
#include "hal.h"

// Includes for user code
#include "status_led.h"
#include "pwm_user.h"
#include "adc_user.h"
#include "tim_user.h"
#include "utils.h"
#include "ext_user.h"

// Includes for the serial connection part
#include "chprintf.h"
#include "usbcfg.h"
#include "shell_user.h"

// Includes for the Internet part
#include "web.h"

virtual_timer_t adc_vt;

#define I2S_BUF_SIZE 4096
static uint16_t i2s_tx_buf[I2S_BUF_SIZE];
static uint32_t sound_index;

static void i2s_cb(I2SDriver* driver, size_t offset, size_t n);

extern uint16_t* _binary_pic_pcm_start;

#define I2SDIV 12
#define ODD 1 << 8
#define MCKOE 1 << 9

static const I2SConfig i2s3_cfg = {
    i2s_tx_buf,
    NULL,
    I2S_BUF_SIZE,
    i2s_cb,
    0b0000000,
    MCKOE | I2SDIV | ODD
};

static void i2s_cb(I2SDriver* driver, size_t offset, size_t n) {
    UNUSED(driver);
    if (offset != n) {// first half of the buffer transmitted
        for (int i = 0; i < I2S_BUF_SIZE / 2; i++)
            i2s_tx_buf[i] = _binary_pic_pcm_start[sound_index + i];
        sound_index += I2S_BUF_SIZE / 2;
    } else if (offset == n) { // second half of the buffer
        for (int i = 0; i < I2S_BUF_SIZE / 2; i++)
            i2s_tx_buf[I2S_BUF_SIZE / 2 + i] = _binary_pic_pcm_start[sound_index + i];
        sound_index += I2S_BUF_SIZE / 2;
    }
}

/*
 * Entry point
 */
int main(void) {

    // Init functions
    halInit();
    chSysInit();

    // Init the PWM
    pwm_init();
    pwm_start(&PWMD10, &pwm_config_tim10);
    pwm_start(&PWMD11, &pwm_config_tim11);
    pwm_start(&PWMD13, &pwm_config_tim13);
    pwm_start(&PWMD14, &pwm_config_tim14);
    update_stat_led();
    pwmStart(&PWMD3, &pwm_config_tim3);
    pwmStart(&PWMD1, &pwm_config_tim1);
    pwmStart(&PWMD4, &pwm_config_tim4);

    // Init the ext interrupt
    ext_init();

    // Init the timer
    chVTObjectInit(&adc_vt);

    // Init the ADC
    adcStart(&ADCD1, NULL);

    // Init the SerialUSB
    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);

    usbDisconnectBus(serusbcfg.usbp);
    //chThdSleepMilliseconds(1500);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    // Init the shell
    shellInit();

    // Init the Internet connection
    web_init();

    // Init the I2S bus
    i2sStart(&I2SD3, &i2s3_cfg);
    for (int i = 0; i < I2S_BUF_SIZE; i++)
        i2s_tx_buf[i] = _binary_pic_pcm_start[i];
    sound_index = I2S_BUF_SIZE;

    palSetPadMode(GPIOA, 15, PAL_MODE_ALTERNATE(6));
    palSetPadMode(GPIOB, 3, PAL_MODE_ALTERNATE(6));
    palSetPadMode(GPIOB, 5, PAL_MODE_ALTERNATE(6));
    palSetPadMode(GPIOC, 7, PAL_MODE_ALTERNATE(6));

    // Start ADC sampling
    chVTSet(&adc_vt, MS2ST(100), adc_vt_cb, NULL);

    // Create the HTTP thread
    chThdCreateStatic(wa_http_server, sizeof(wa_http_server), NORMALPRIO + 1,
                http_function, NULL);

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
