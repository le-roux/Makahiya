#include "ch.h"
#include "hal.h"

#include "RTT_streams.h"
#include "pwmdriver.h"
#include "bluetooth.h"
#include "wifi.h"
#include "websocket.h"
#include "sound.h"
#include "alarm.h"

int main(void) {

	halInit();
	chSysInit();

	RTTObjectInit(&RTTD, 0);

	pwmUserInit();
	bluetoothInit();
	wifiInit();
	audioInit();
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

	/**
	 * Lighting LEDs to show end of startup + sound
	 */

	music_id = 3;
	repeat = 1;
	chBSemSignal(&audio_bsem);

	setLed(LED1_R, 12);
	setLed(LED2_R, 12);
	setLed(LED3_R, 12);
	setLed(LED4_R, 12);
	setLed(LED5_R, 12);
	setLed(LED_HP_R, 12);
	chThdSleepMilliseconds(1000);
	setLed(LED1_G, 12);
	setLed(LED2_G, 12);
	setLed(LED3_G, 12);
	setLed(LED4_G, 12);
	setLed(LED5_G, 12);
	setLed(LED_HP_G, 12);
	setLed(LED1_R, 0);
	setLed(LED2_R, 0);
	setLed(LED3_R, 0);
	setLed(LED4_R, 0);
	setLed(LED5_R, 0);
	setLed(LED_HP_R, 0);
	chThdSleepMilliseconds(1000);
	setLed(LED1_B, 12);
	setLed(LED2_B, 12);
	setLed(LED3_B, 12);
	setLed(LED4_B, 12);
	setLed(LED5_B, 12);
	setLed(LED_HP_B, 12);
	setLed(LED1_G, 0);
	setLed(LED2_G, 0);
	setLed(LED3_G, 0);
	setLed(LED4_G, 0);
	setLed(LED5_G, 0);
	setLed(LED_HP_G, 0);
	chThdSleepMilliseconds(1000);
	setLed(LED1_B, 0);
	setLed(LED2_B, 0);
	setLed(LED3_B, 0);
	setLed(LED4_B, 0);
	setLed(LED5_B, 0);
	setLed(LED_HP_B, 0);

	repeat = 0;

	while(true) {
		chThdSleepMilliseconds(1000);
	}
}
