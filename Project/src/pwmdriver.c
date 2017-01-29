#include "ch.h"
#include "hal.h"
#include "pwmdriver.h"
#include "utils.h"

int VALUES[100];
static volatile int softPwmEnabled[11] = {0};
static PWMDriver * pwmd[8] = {&PWMD1, &PWMD2, &PWMD3, &PWMD4, NULL, NULL, NULL, &PWMD8};
static virtual_timer_t servo[7];

static void clearServo1(void * arg){
	UNUSED(arg);
	palClearPad(GPIOA, 4);
}

static void clearServo2(void * arg){
	UNUSED(arg);
	palClearPad(GPIOA, 5);
}

static void clearServo3(void * arg){
	UNUSED(arg);
	palClearPad(GPIOA, 6);
}

static void clearServo4(void * arg){
	UNUSED(arg);
	palClearPad(GPIOA, 7);
}

static void clearServo5(void * arg){
	UNUSED(arg);
	palClearPad(GPIOC, 4);
}

static void setServosI(void * arg){
	UNUSED(arg);
	chSysLockFromISR();
	if (softPwmEnabled[6]){
		palSetPad(GPIOA, 4);
		chVTSetI(&(servo[1]), US2ST(softPwmEnabled[6]), clearServo1, NULL);
	}
	if (softPwmEnabled[7]){
		palSetPad(GPIOA, 5);
		chVTSetI(&(servo[2]), US2ST(softPwmEnabled[7]), clearServo2, NULL);
	}
	if (softPwmEnabled[8]){
		palSetPad(GPIOA, 6);
		chVTSetI(&(servo[3]), US2ST(softPwmEnabled[8]), clearServo3, NULL);
	}
	if (softPwmEnabled[9]){
		palSetPad(GPIOA, 7);
		chVTSetI(&(servo[4]), US2ST(softPwmEnabled[9]), clearServo4, NULL);
	}
	if (softPwmEnabled[10]){
		palSetPad(GPIOC, 4);
		chVTSetI(&(servo[5]), US2ST(softPwmEnabled[10]), clearServo5, NULL);
	}
	chVTSetI(&(servo[0]), MS2ST(20), setServosI, NULL);
	chSysUnlockFromISR();
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

void pwmUserInit(void){

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

	for(int i = 0 ; i < 6 ; i ++)
		chVTObjectInit(&servo[i]);

	chVTSet(&(servo[0]), MS2ST(20), setServosI, NULL);

	for (int i = 0; i < 100; i++)
		VALUES[i] = 0;
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

void setLedHP(int R, int G, int B, int W){
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

void shakeServo(int id, int n){
	for(int i = 0 ; i < n ; i ++){
		setServo(id, 2900);
		chThdSleepMilliseconds(1000);
		setServo(id, 800);
		chThdSleepMilliseconds(1000);
	}
}

void setValue(int varId, int value) {
    VALUES[varId] = value;
    switch(varId) {
        case(LED1_ON): {
            if (value)
                setLedRGB(1, VALUES[LED1_R], VALUES[LED1_G], VALUES[LED1_B]);
            else
                setLedRGB(1, 0, 0, 0);
            break;
        }
        case(LED2_ON): {
            if (value)
                setLedRGB(2, VALUES[LED2_R], VALUES[LED2_G], VALUES[LED2_B]);
            else
                setLedRGB(2, 0, 0, 0);
            break;
        }
        case(LED3_ON): {
            if (value)
                setLedRGB(3, VALUES[LED3_R], VALUES[LED3_G], VALUES[LED3_B]);
            else
                setLedRGB(3, 0, 0, 0);
            break;
        }
        case(LED4_ON): {
            if (value)
                setLedRGB(4, VALUES[LED4_R], VALUES[LED4_G], VALUES[LED4_B]);
            else
                setLedRGB(4, 0, 0, 0);
            break;
        }
        case(LED5_ON): {
            if (value)
                setLedRGB(5, VALUES[LED5_R], VALUES[LED5_G], VALUES[LED5_B]);
            else
                setLedRGB(5, 0, 0, 0);
            break;
        }
        case(LED_HP_ON): {
            if (value)
                setLedHP(VALUES[LED_HP_R], VALUES[LED_HP_G], VALUES[LED_HP_B], VALUES[LED_HP_W]);
            else
                setLedHP(0, 0, 0, 0);
            break;
        }
        default: {
            if (IS_LED_1(varId) && VALUES[LED1_ON])
                setLed(varId, value);
            else if (IS_LED_2(varId) && VALUES[LED2_ON])
                setLed(varId, value);
            else if (IS_LED_2(varId) && VALUES[LED2_ON])
                setLed(varId, value);
            else if (IS_LED_3(varId) && VALUES[LED3_ON])
                setLed(varId, value);
            else if (IS_LED_4(varId) && VALUES[LED4_ON])
                setLed(varId, value);
            else if (IS_LED_5(varId) && VALUES[LED5_ON])
                setLed(varId, value);
            else if (IS_LED_HP(varId) && VALUES[LED_HP_ON])
                setLed(varId, value);
            else if (IS_SERVO(varId))
                setServo(varId - SERVO_BASE, value);
        }
    }
}

int getValue(int varId) {
    return VALUES[varId];
}
