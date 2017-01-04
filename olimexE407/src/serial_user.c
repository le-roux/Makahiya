#include "serial_user.h"

SerialConfig serial_cfg = {
	115200,
	0,
	0,
	0x00000300
};

SerialDriver* bluetooth_SD = &SD3;

uint8_t serial_tx_buffer[SERIAL_TX_BUFFER_SIZE];
uint8_t serial_rx_buffer[SERIAL_RX_BUFFER_SIZE];

void serial_set_pin(void) {
	palSetPadMode(GPIOD, 0, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPad(GPIOD, 0);
	palSetPadMode(GPIOD, 8, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOD, 9, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOD, 11, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOD, 12, PAL_MODE_ALTERNATE(7));
	chThdSleepMilliseconds(50);
	palClearPad(GPIOD, 0);
	chThdSleepMilliseconds(50);
	palSetPad(GPIOD, 0);
	sdStart(bluetooth_SD, &serial_cfg);
}
