#include "ch.h"
#include "hal.h"

#include "RTT_streams.h"
#include "pwmdriver.h"
#include "fdc2214.h"
#include "bluetooth.h"
#include "wifi.h"
#include "websocket.h"
#include "sound.h"

int main(void) {

	halInit();
	chSysInit();

	RTTObjectInit(&RTTD, 0);
	initBluetooth();
	wifi_init();
	chThdCreateStatic(wa_websocket, sizeof(wa_websocket), NORMALPRIO + 1, websocket, "0");
	fdc_init();
	init_audio();

	initPwm();
	setLed(LED1_G, 12);
	setLed(LED2_R, 12);
	setLed(LED4_B, 12);

	chThdCreateStatic(wa_audio, sizeof(wa_audio), NORMALPRIO, audio_playback, NULL);
	chThdCreateStatic(wa_flash, sizeof(wa_flash), NORMALPRIO, flash_audio_in, NULL);
	music_id = 0;
	repeat = 1;
	chBSemSignal(&audio_bsem);

	shakeServo(3, 5);

	while(true) {
		chThdSleepMilliseconds(1000);
	}
}
