#include "ch.h"
#include "hal.h"

#include "RTT_streams.h"
#include "pwmdriver.h"
#include "fdc2214.h"
#include "bluetooth.h"
#include "wifi.h"
#include "websocket.h"

int main(void) {

	halInit();
	chSysInit();

	RTTObjectInit(&RTTD, 0);
	initBluetooth();
	wifi_init();
	chThdCreateStatic(wa_websocket, sizeof(wa_websocket), NORMALPRIO + 1, websocket, "0");
	//fdc_init();

	initPwm();
	setLed(LED1_G, 12);
	setLed(LED2_R, 12);
	setLed(LED4_B, 12);

	shakeServo(3, 5);

	while(true) {
		chThdSleepMilliseconds(1000);
	}
}
