// ChibiOS includes
#include "ch.h"
#include "hal.h"

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

	// Init the SerialUSB
	sduObjectInit(&SDU1);
	sduStart(&SDU1, &serusbcfg);

	usbDisconnectBus(serusbcfg.usbp);
	usbStart(serusbcfg.usbp, &usbcfg);
	usbConnectBus(serusbcfg.usbp);

	// Init the serial
	serial_set_pin();
	initBluetooth();

	// Loop forever.
	while (true) {
		chThdSleepMilliseconds(1000);
	}

	return 0;
}
