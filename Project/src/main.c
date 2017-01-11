#include "ch.h"
#include "hal.h"

#include "leds.h"

int main(void) {

	halInit();
	chSysInit();
	
	initLeds();
	setLed(LED1_R, 32);
	setLed(LED4_R, 32);

	while(true){
		chThdSleepMilliseconds(1000);
	}
}
