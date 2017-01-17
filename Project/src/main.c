#include "ch.h"
#include "hal.h"

#include "pwmdriver.h"

int main(void) {

	halInit();
	chSysInit();
	
	initPwm();
	setLed(LED1_R, 32);
	setLed(LED4_R, 32);

	while(true){
		chThdSleepMilliseconds(1000);
	}
}
