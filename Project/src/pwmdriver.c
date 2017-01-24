#include "ch.h"
#include "hal.h"
#include "pwmdriver.h"
#include "utils.h"

static volatile char softPwmEnabled[11] = {0};
static PWMDriver * pwmd[8] = {&PWMD1, &PWMD2, &PWMD3, &PWMD4, NULL, NULL, NULL, &PWMD8};
static virtual_timer_t servo[7];

static void setServos(void * arg);

static void clearServo1(void * arg){
	UNUSED(arg);
	palClearPad(GPIOA, 4);
}

static void clearServo2(void * arg){
	UNUSED(arg);
	palClearPad(GPIOC, 4);
}

static void clearServo3(void * arg){
	UNUSED(arg);
	palClearPad(GPIOC, 5);
}

static void clearServo4(void * arg){
	UNUSED(arg);
	palClearPad(GPIOC, 6);
}

static void clearServo5(void * arg){
	UNUSED(arg);
	palClearPad(GPIOC, 7);
}

static void setServosI(void * arg){
	UNUSED(arg);
	chSysLockFromISR();
	if (softPwmEnabled[6]){
		palSetPad(GPIOA, 4);
		chVTSetI(&(servo[1]), softPwmEnabled[6], clearServo1, NULL);
	}
	if (softPwmEnabled[7]){
		palSetPad(GPIOC, 4);
		chVTSetI(&(servo[2]), softPwmEnabled[7], clearServo2, NULL);
	}
	if (softPwmEnabled[8]){
		palSetPad(GPIOC, 5);
		chVTSetI(&(servo[3]), softPwmEnabled[8], clearServo3, NULL);
	}
	if (softPwmEnabled[9]){
		palSetPad(GPIOC, 6);
		chVTSetI(&(servo[4]), softPwmEnabled[9], clearServo4, NULL);
	}
	if (softPwmEnabled[10]){
		palSetPad(GPIOC, 7);
		chVTSetI(&(servo[5]), softPwmEnabled[10], clearServo5, NULL);
	}
	chVTSetI(&(servo[0]), MS2ST(20), setServosI, NULL);
	chSysUnlockFromISR();
}

static void setServos(void * arg){
	UNUSED(arg);
	if (softPwmEnabled[6]){
		palSetPad(GPIOA, 4);
		chVTSet(&(servo[1]), softPwmEnabled[6], clearServo1, NULL);
	}
	if (softPwmEnabled[7]){
		palSetPad(GPIOC, 4);
		chVTSet(&(servo[2]), softPwmEnabled[7], clearServo2, NULL);
	}
	if (softPwmEnabled[8]){
		palSetPad(GPIOC, 5);
		chVTSet(&(servo[3]), softPwmEnabled[8], clearServo3, NULL);
	}
	if (softPwmEnabled[9]){
		palSetPad(GPIOC, 6);
		chVTSet(&(servo[4]), softPwmEnabled[9], clearServo4, NULL);
	}
	if (softPwmEnabled[10]){
		palSetPad(GPIOC, 7);
		chVTSet(&(servo[5]), softPwmEnabled[10], clearServo5, NULL);
	}
	chVTSet(&(servo[0]), MS2ST(20), setServosI, NULL);
}

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

void initPwm(void){

	// LED 1

	palSetPadMode(GPIOC, 5,  PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(GPIOC, 5);
	palSetPadMode(GPIOB, 0,  PAL_MODE_ALTERNATE(2));
	palClearPad(GPIOB, 0);
	palSetPadMode(GPIOB, 1,  PAL_MODE_ALTERNATE(2));
	palClearPad(GPIOB, 1);

	// LED 2

	palSetPadMode(GPIOB, 2,  PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(GPIOB, 2);
	palSetPadMode(GPIOB, 10, PAL_MODE_ALTERNATE(1));
	palClearPad(GPIOB, 10);
	palSetPadMode(GPIOB, 11, PAL_MODE_ALTERNATE(1));
	palClearPad(GPIOB, 11);

	// LED 3

	palSetPadMode(GPIOB, 14, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(GPIOB, 14);
	palSetPadMode(GPIOC, 7,  PAL_MODE_ALTERNATE(3));
	palClearPad(GPIOC, 7);
	palSetPadMode(GPIOC, 8,  PAL_MODE_ALTERNATE(3));
	palClearPad(GPIOC, 8);

	// LED 4

	palSetPadMode(GPIOC, 9,  PAL_MODE_ALTERNATE(3));
	palClearPad(GPIOC, 9);
	palSetPadMode(GPIOA, 8,  PAL_MODE_ALTERNATE(1));
	palClearPad(GPIOA, 8);
	palSetPadMode(GPIOC, 11, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(GPIOC, 11);

	// LED 5

	palSetPadMode(GPIOC, 12, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(GPIOC, 12);
	palSetPadMode(GPIOD, 2,  PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(GPIOD, 2);
	palSetPadMode(GPIOB, 3,  PAL_MODE_ALTERNATE(1));
	palClearPad(GPIOB, 3);

	// LED_HP

	palSetPadMode(GPIOB, 4,  PAL_MODE_ALTERNATE(2));
	palClearPad(GPIOB, 4);
	palSetPadMode(GPIOB, 5,  PAL_MODE_ALTERNATE(2));
	palClearPad(GPIOB, 5);
	palSetPadMode(GPIOC, 1,  PAL_MODE_ALTERNATE(2));
	palClearPad(GPIOC, 1);
	palSetPadMode(GPIOC, 2,  PAL_MODE_ALTERNATE(2));
	palClearPad(GPIOC, 2);

	// Servos

	palSetPadMode(GPIOC, 4, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(GPIOC, 4);
	palSetPadMode(GPIOA, 4, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(GPIOA, 4);
	palSetPadMode(GPIOA, 5, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(GPIOA, 5);
	palSetPadMode(GPIOA, 6, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(GPIOA, 6);
	palSetPadMode(GPIOA, 7, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(GPIOA, 7);

	// Timers

	pwmStart(&PWMD1, &pwmconfig1);
	pwmStart(&PWMD2, &pwmconfig2);
	pwmStart(&PWMD3, &pwmconfig);
	pwmStart(&PWMD4, &pwmconfig4);
	pwmStart(&PWMD8, &pwmconfig);

	pwmEnablePeriodicNotification(&PWMD1);
	pwmEnablePeriodicNotification(&PWMD2);
	pwmEnablePeriodicNotification(&PWMD4);

	setServos(NULL);
}

void setLed(int led, int power){

	int soft = 0;

	if (led > 32) {

		//SOFT PWM

		softPwmEnabled[led - 33] = power;
		if(led < 37)
			led -= 9;
		led -= 23;
		soft = 1;

	}

	pwmEnableChannel(pwmd[led/4], led % 4, power);
	if (soft)
		pwmEnableChannelNotification(pwmd[led/4], led % 4);

}

void setLedHp(int R, int G, int B, int W){
	setLed(LED_HP_R, R);
	setLed(LED_HP_G, G);
	setLed(LED_HP_B, B);
	setLed(LED_HP_W, W);
}

void setLedRGB(int id, int R, int G, int B){
	switch(id){
		case 1:
			setLed(LED1_R, R);
			setLed(LED1_G, G);
			setLed(LED1_B, B);
			break;
		case 2:
			setLed(LED2_R, R);
			setLed(LED2_G, G);
			setLed(LED2_B, B);
			break;
		case 3:
			setLed(LED3_R, R);
			setLed(LED3_G, G);
			setLed(LED3_B, B);
			break;
		case 4:
			setLed(LED4_R, R);
			setLed(LED4_G, G);
			setLed(LED4_B, B);
			break;
		case 5:
			setLed(LED5_R, R);
			setLed(LED5_G, G);
			setLed(LED5_B, B);
			break;
	}
}

void setServo(int id, int value){
	softPwmEnabled[5+id] = value;
}
