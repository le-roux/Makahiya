#include "hal.h"

#include "i2c_user.h"

I2CConfig i2c2_cfg = {
    OPMODE_I2C,
    350000,
    FAST_DUTY_CYCLE_2
};

uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t tx_buffer[TX_BUFFER_SIZE];

void i2c_set_pins(void) {
    palSetPadMode(GPIOF, 0, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
    palSetPadMode(GPIOF, 1, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
    palSetPadMode(GPIOF, 3, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOF, 4, PAL_MODE_INPUT);
    palSetPadMode(GPIOF, 5, PAL_MODE_OUTPUT_PUSHPULL);

    // Set ADDR pin to 0 -> addr = 0x2A
    palClearPad(GPIOF, 3);

    // Set SD pin to 0
    palClearPad(GPIOF, 5);
}
