#include "hal.h"

#include "i2c_user.h"

I2CConfig i2c1_cfg = {
	OPMODE_I2C,
	350000,
	FAST_DUTY_CYCLE_2
};

uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t tx_buffer[TX_BUFFER_SIZE];

void i2c_set_pins(void) {

	// I2C

	palSetPadMode(GPIOB, 8, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
	palSetPadMode(GPIOB, 9, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);

	// Interrupts

	palSetPadMode(GPIOC, 10, PAL_MODE_INPUT);
	palSetPadMode(GPIOC, 13, PAL_MODE_INPUT);
	
	// Shutdown

	palSetPadMode(GPIOC, 14, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOC, 15, PAL_MODE_OUTPUT_PUSHPULL);

	// Set Shutdown pin to 0

	palClearPad(GPIOC, 14);
	palClearPad(GPIOC, 15);
}
