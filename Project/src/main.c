#include "ch.h"
#include "hal.h"

#include "RTT_streams.h"
#include "pwmdriver.h"
#include "fdc2214.h"
#include "bluetooth.h"
#include "wifi.h"
#include "websocket.h"
#include "sound.h"
#include "alarm.h"

int main(void) {

	halInit();
	chSysInit();

	RTTObjectInit(&RTTD, 0);

	bluetoothInit();
	wifiInit();
	fdcInit();
	audioInit();
	pwmUserInit();
	alarmInit();

	/**
	 * Websocket thread
	 */
	chThdCreateStatic(wa_websocket, sizeof(wa_websocket), NORMALPRIO + 1, websocket, "0");

	/**
	 * Audio threads
	 */
	chThdCreateStatic(wa_audio, sizeof(wa_audio), NORMALPRIO, audio_playback, NULL);
	chThdCreateStatic(wa_audio_in, sizeof(wa_audio_in), NORMALPRIO + 1, wifi_audio_in, NULL);
	chThdCreateStatic(wa_flash, sizeof(wa_flash), NORMALPRIO, flash_audio_in, NULL);

	while(true) {
		chThdSleepMilliseconds(1000);
	}
}
