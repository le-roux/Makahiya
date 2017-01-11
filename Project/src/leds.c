#include "ch.h"
#include "hal.h"
#include "leds.h"
#include "utils.h"

static char softPwmEnabled[6];

static void tim1cb (PWMDriver *pwmd){
	UNUSED(pwmd);
	if (softPwmEnabled[0])
		palSetPad(GPIOC, 5);
	if (softPwmEnabled[1])
		palSetPad(GPIOB, 2);
	if (softPwmEnabled[2])
		palSetPad(GPIOB, 14);
}

static void softpwm0(PWMDriver * pwmd){
	UNUSED(pwmd);
	palClearPad(GPIOC, 5);
}

static void softpwm1(PWMDriver * pwmd){
	UNUSED(pwmd);
	palClearPad(GPIOB, 2);
}

static void softpwm2(PWMDriver * pwmd){
	UNUSED(pwmd);
	palClearPad(GPIOB, 14);
}

static void tim2cb (PWMDriver *pwmd){
	UNUSED(pwmd);
	if (softPwmEnabled[3])
		palSetPad(GPIOC, 11);
}

static void softpwm3(PWMDriver * pwmd){
	UNUSED(pwmd);
	palClearPad(GPIOC, 11);
}

static void tim4cb (PWMDriver *pwmd){
	UNUSED(pwmd);
	if (softPwmEnabled[4])
		palSetPad(GPIOC, 12);
	if (softPwmEnabled[5])
		palSetPad(GPIOD, 2);
}

static void softpwm4(PWMDriver * pwmd){
	UNUSED(pwmd);
	palClearPad(GPIOC, 12);
}

static void softpwm5(PWMDriver * pwmd){
	UNUSED(pwmd);
	palClearPad(GPIOD, 2);
}

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

static const PWMConfig pwmconfig1 = {
	100000,
	256,
	tim1cb,
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, softpwm0},
		{PWM_OUTPUT_ACTIVE_HIGH, softpwm1},
		{PWM_OUTPUT_ACTIVE_HIGH, softpwm2},
	},
	0,
	0,
};

static const PWMConfig pwmconfig2 = {
	100000,
	256,
	tim2cb,
	{
		{PWM_OUTPUT_ACTIVE_HIGH, softpwm3},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
	},
	0,
	0,
};

static const PWMConfig pwmconfig4 = {
	100000,
	256,
	tim4cb,
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, softpwm4},
		{PWM_OUTPUT_ACTIVE_HIGH, softpwm5},
	},
	0,
	0,
};

void initLeds(void){

	// LED 1

	palSetPadMode(GPIOC, 5,  PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOB, 0,  PAL_MODE_ALTERNATE(2));
	palSetPadMode(GPIOB, 1,  PAL_MODE_ALTERNATE(2));

	// LED 2

	palSetPadMode(GPIOB, 2,  PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOB, 10, PAL_MODE_ALTERNATE(1));
	palSetPadMode(GPIOB, 11, PAL_MODE_ALTERNATE(1));

	// LED 3

	palSetPadMode(GPIOB, 14, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOC, 7,  PAL_MODE_ALTERNATE(3));
	palSetPadMode(GPIOC, 8,  PAL_MODE_ALTERNATE(3));

	// LED 4

	palSetPadMode(GPIOC, 9,  PAL_MODE_ALTERNATE(3));
	palSetPadMode(GPIOA, 8,  PAL_MODE_ALTERNATE(1));
	palSetPadMode(GPIOC, 11, PAL_MODE_OUTPUT_PUSHPULL);

	// LED 5

	palSetPadMode(GPIOC, 12, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOD, 2,  PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOC, 2,  PAL_MODE_ALTERNATE(1));

	// LED_HP

	palSetPadMode(GPIOB, 4,  PAL_MODE_ALTERNATE(2));
	palSetPadMode(GPIOB, 5,  PAL_MODE_ALTERNATE(2));
	palSetPadMode(GPIOC, 1,  PAL_MODE_ALTERNATE(2));
	palSetPadMode(GPIOC, 2,  PAL_MODE_ALTERNATE(2));

	// Timers

	pwmStart(&PWMD1, &pwmconfig1);
	pwmStart(&PWMD2, &pwmconfig2);
	pwmStart(&PWMD3, &pwmconfig);
	pwmStart(&PWMD4, &pwmconfig4);
	pwmStart(&PWMD8, &pwmconfig);
}

void setLed(int led, int power){

	if (led > 32) {

		//SOFT PWM

		softPwmEnabled[led - 33] = power;
		if(led < 37)
			led -= 9;
		led -= 23;

	}

	PWMDriver * pwmd;
	switch(led / 4){
		case 0:
			pwmd = &PWMD1;
			break;
		case 1:
			pwmd = &PWMD2;
			break;
		case 2:
			pwmd = &PWMD3;
			break;
		case 3:
			pwmd = &PWMD4;
			break;
		case 7:
			pwmd = &PWMD8;
			break;
	}

	pwmEnableChannel(pwmd, led % 4, power);

}

void setLedHp(int R, int G, int B, int W){
	setLed(LED_HP_R, R);
	setLed(LED_HP_G, G);
	setLed(LED_HP_B, B);
	setLed(LED_HP_W, W);
}

void setLed1(int R, int G, int B){
	setLed(LED1_R, R);
	setLed(LED1_G, G);
	setLed(LED1_B, B);
}

void setLed2(int R, int G, int B){
	setLed(LED2_R, R);
	setLed(LED2_G, G);
	setLed(LED2_B, B);
}

void setLed3(int R, int G, int B){
	setLed(LED3_R, R);
	setLed(LED3_G, G);
	setLed(LED3_B, B);
}

void setLed4(int R, int G, int B){
	setLed(LED4_R, R);
	setLed(LED4_G, G);
	setLed(LED4_B, B);
}

void setLed5(int R, int G, int B){
	setLed(LED5_R, R);
	setLed(LED5_G, G);
	setLed(LED5_B, B);
}
