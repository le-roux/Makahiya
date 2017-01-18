#include "ch.h"
#include "hal.h"
#include "bluetooth.h"
#include "utils.h"
#include "chprintf.h"
#include <string.h>

static SerialDriver* bluetooth_SD = &SD2;
RTTStream RTTD;

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
		chprintf((BaseSequentialStream *)&RTTD, "%c", c[0]);
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

void initBluetooth(void){

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
	chThdSleepMilliseconds(10000);
	startSerialShell();
	chThdSleepMilliseconds(1000);
	
	// Configuring the RN-42

	uint8_t buf[16] = "$$$";
	chprintf((BaseSequentialStream *)&RTTD, "$$$\n");
	sdWrite(bluetooth_SD, buf, 3);
	chThdSleepMilliseconds(100);
	strncpy((char *) buf, "SO,Log : \r", 16);
	chprintf((BaseSequentialStream *)&RTTD, "SO,Log : \n");
	sdWrite(bluetooth_SD, buf, 10);
	chThdSleepMilliseconds(100);
	strncpy((char *) buf, "SA,0\r", 16);
	chprintf((BaseSequentialStream *)&RTTD, "SA,0\n");
	sdWrite(bluetooth_SD, buf, 5);
	chThdSleepMilliseconds(100);
	strncpy((char *) buf, "SN,Makahiya\r", 16);
	chprintf((BaseSequentialStream *)&RTTD, "SN,Makahiya\n");
	sdWrite(bluetooth_SD, buf, 12);
	chThdSleepMilliseconds(100);
	strncpy((char *) buf, "Q,0\r", 16);
	chprintf((BaseSequentialStream *)&RTTD, "Q,0\n");
	sdWrite(bluetooth_SD, buf, 4);
	chThdSleepMilliseconds(100);
	strncpy((char *) buf, "---\r", 16);
	chprintf((BaseSequentialStream *)&RTTD, "---\n");
	sdWrite(bluetooth_SD, buf, 4);
	chThdSleepMilliseconds(100);
}
