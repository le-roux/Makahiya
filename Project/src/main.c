#include "ch.h"
#include "hal.h"

#include "RTT_streams.h"
#include "pwmdriver.h"
#include "fdc2214.h"
#include "bluetooth.h"

int main(void) {

	halInit();
	chSysInit();

	/*initPwm();
	setLed(LED1_R, 32);
	setLed(LED4_R, 32);*/

	RTTObjectInit(&RTTD, 0);
	fdc_init();
	//initBluetooth();

	while(true) {
		chThdSleepMilliseconds(1000);
	}
}
