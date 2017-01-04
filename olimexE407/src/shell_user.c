#include "shell_user.h"
#include "utils.h"
#include "usbcfg.h"
#include "chprintf.h"
#include "serial_user.h"

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

void startSerialShell(void){
	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
	chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);
}
