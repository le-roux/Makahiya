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
    msg_t status;
    chThdSleepMilliseconds(10);
    tx_buffer[0] = 0x7F;
    tx_buffer[1] = DEVICE_ID_H;
    tx_buffer[2] = DEVICE_ID_L;
    status = i2cMasterTransmitTimeout(&I2CD2, FDC1_ADDR, tx_buffer, 1, rx_buffer, 2, MS2ST(4));
    if (status != MSG_OK)
        chprintf((BaseSequentialStream*)&SDU1, "failure\r\n");
    else
        chprintf((BaseSequentialStream*)&SDU1, "read: %x\r\n", rx_buffer);
    if (i2cGetErrors(&I2CD2) & I2C_ACK_FAILURE)
        chprintf((BaseSequentialStream*)&SDU1, "ack failure\r\n");

    read_register(FDC1_ADDR, CONFIG_REG_ADDR);
    config &= ~SLEEP_MODE;
    write_register(FDC1_ADDR, CONFIG_REG_ADDR, config);
    read_register(FDC1_ADDR, CONFIG_REG_ADDR);
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
