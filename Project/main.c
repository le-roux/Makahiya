#include "ch.h"
#include "hal.h"

static const PWMConfig pwmconfig = {
	100000,
	256,
	NULL,
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
	},
	0,
	0,
};

int main(void) {

	halInit();
	chSysInit();

	palSetPadMode(GPIOB, 0, PAL_MODE_ALTERNATE(2));
	pwmStart(&PWMD3, &pwmconfig);
	pwmEnableChannel(&PWMD3, 2, 32);

	while(true){
		chThdSleepMilliseconds(1000);
	}
}
