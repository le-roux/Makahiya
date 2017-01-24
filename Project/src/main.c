#include "ch.h"
#include "hal.h"

#include "RTT_streams.h"
#include "pwmdriver.h"
#include "fdc2214.h"
#include "bluetooth.h"

int main(void) {

	halInit();
	chSysInit();

	RTTObjectInit(&RTTD, 0);
	fdc_init();
	initBluetooth();

	initPwm();
	setLed(LED1_G, 12);
	setLed(LED2_R, 12);
	setLed(LED4_B, 12);

	while(true) {
		chThdSleepMilliseconds(1000);
	}
}
