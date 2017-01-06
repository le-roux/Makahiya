#include "shell_user.h"
#include "utils.h"
#include "usbcfg.h"
#include "chprintf.h"
#include "serial_user.h"
#include <string.h>

static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {
	UNUSED(arg);
	char c;
	while(1){
		c = sdGet(bluetooth_SD);
		chprintf((BaseSequentialStream *)&SDU1, "%c", c);
	}	
}

static THD_WORKING_AREA(waThread2, 128);
static THD_FUNCTION(Thread2, arg) {
	UNUSED(arg);
	uint8_t c[1];
	while(1){
		c[0] = chSequentialStreamGet(&SDU1);
		chprintf((BaseSequentialStream *)&SDU1, "%c", c[0]);
		sdWrite(bluetooth_SD, c, 1);
	}	
}

static void startSerialShell(void){
	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
	chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);
}

void initBluetooth(void){
	chThdSleepMilliseconds(10000);
	startSerialShell();
	chThdSleepMilliseconds(1000);
	uint8_t buf[16] = "$$$";
	chprintf((BaseSequentialStream *)&SDU1, "$$$\n");
	sdWrite(bluetooth_SD, buf, 3);
	chThdSleepMilliseconds(100);
	strncpy((char *) buf, "SO,Log : \r", 16);
	chprintf((BaseSequentialStream *)&SDU1, "SO,Log : \n");
	sdWrite(bluetooth_SD, buf, 10);
	chThdSleepMilliseconds(100);
	strncpy((char *) buf, "SA,0\r", 16);
	chprintf((BaseSequentialStream *)&SDU1, "SA,0\n");
	sdWrite(bluetooth_SD, buf, 5);
	chThdSleepMilliseconds(100);
	strncpy((char *) buf, "SN,Makahiya\r", 16);
	chprintf((BaseSequentialStream *)&SDU1, "SN,Makahiya\n");
	sdWrite(bluetooth_SD, buf, 12);
	chThdSleepMilliseconds(100);
	strncpy((char *) buf, "Q,0\r", 16);
	chprintf((BaseSequentialStream *)&SDU1, "Q,0\n");
	sdWrite(bluetooth_SD, buf, 4);
	chThdSleepMilliseconds(100);
	strncpy((char *) buf, "---\r", 16);
	chprintf((BaseSequentialStream *)&SDU1, "---\n");
	sdWrite(bluetooth_SD, buf, 4);
	chThdSleepMilliseconds(100);
}
