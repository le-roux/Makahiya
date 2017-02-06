#include "ch.h"
#include "hal.h"
#include "bluetooth.h"
#include "RTT_streams.h"
#include "utils.h"
#include "chprintf.h"
#include <string.h>

static SerialDriver* bluetooth_SD = &SD2;

static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {
	UNUSED(arg);
	char c;
	while(1){
		c = sdGet(bluetooth_SD);
		chprintf((BaseSequentialStream *)&RTTD, "%c", c);
	}
}

static THD_WORKING_AREA(waThread2, 128);
static THD_FUNCTION(Thread2, arg) {
	UNUSED(arg);
	uint8_t c[1];
	while(1){
		c[0] = chSequentialStreamGet((BaseSequentialStream *) &RTTD);
		sdWrite(bluetooth_SD, c, 1);
	}
}

static void startSerialShell(void){
	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
	chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);
}

static SerialConfig serial_cfg = {
	115200,
	0,
	0,
	0x00000300
};

// Send a message to the bluetooth module as well as a copy to the RTTD debug
// channel. Afterwards, wait for 100ms before returning.
static void send(const char *msg){
	size_t len = strlen(msg);
	const uint8_t *umsg = (const uint8_t *)msg;

	// Send a copy to the RTT for debugging purpose, omitting the final '\r'.
	chSequentialStreamWrite((BaseSequentialStream *)&RTTD, umsg,
		msg[len-1] == '\r' ? len-1 : len);
	chSequentialStreamWrite((BaseSequentialStream *)&RTTD, (const uint8_t *)"\n",
		1);

	sdWrite(bluetooth_SD, umsg, len);
	chThdSleepMilliseconds(100);
}

void bluetoothInit(void){

	// Setting the pins and resetting the RN-42

	palSetPadMode(GPIOD, 0, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPad(GPIOD, 0);
	palSetPadMode(GPIOA, 0, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOA, 1, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));
	chThdSleepMilliseconds(50);
	palClearPad(GPIOD, 0);
	chThdSleepMilliseconds(50);
	palSetPad(GPIOD, 0);

	// Starting the UART

	sdStart(bluetooth_SD, &serial_cfg);
	chThdSleepMilliseconds(1000);
	startSerialShell();
	chThdSleepMilliseconds(1000);

	// Configuring the RN-42

  send("$$$");
	send("SO,Log : \r");
	send("SA,0\r");
	send("S-,Makahiya\r");
	send("Q,0\r");
	send("---\r");
}
