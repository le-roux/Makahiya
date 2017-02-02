#ifndef PWM_H
#define PWM_H

#define LED1_R 33U
#define LED1_G 10U
#define LED1_B 11U
#define LED1_ON 50U

#define LED2_R 34U
#define LED2_G 6U
#define LED2_B 7U
#define LED2_ON 51U

#define LED3_R 35U
#define LED3_G 29U
#define LED3_B 30U
#define LED3_ON 52U

#define LED4_R 31U
#define LED4_G 0U
#define LED4_B 36U
#define LED4_ON 53U

#define LED5_R 37U
#define LED5_G 38U
#define LED5_B 5U
#define LED5_ON 54U

#define LED_HP_R 8U
#define LED_HP_G 9U
#define LED_HP_B 12U
#define LED_HP_W 13U
#define LED_HP_ON 55U

#define IS_LED_1(x) (x == LED1_R || x == LED1_G || x == LED1_B)
#define IS_LED_2(x) (x == LED2_R || x == LED2_G || x == LED2_B)
#define IS_LED_3(x) (x == LED3_R || x == LED3_G || x == LED3_B)
#define IS_LED_4(x) (x == LED4_R || x == LED4_G || x == LED4_B)
#define IS_LED_5(x) (x == LED5_R || x == LED5_G || x == LED5_B)
#define IS_LED_HP(x) (x == LED_HP_R || x == LED_HP_G || x == LED_HP_B || x == LED_HP_W)

#define SERVO_BASE 60U
#define SERVO_1 61U
#define SERVO_2 62U
#define SERVO_3 63U
#define SERVO_4 64U
#define SERVO_5 65U
#define IS_SERVO(x) (x > 60 && x < 66)

extern int VALUES[100];

void pwmUserInit(void);
void setLed(int led, int power);
void setLedI(int led, int power);

void setLedHP(int R, int G, int B, int W);
void setLedHPI(int R, int G, int B, int W);
void setLedRGB(int id, int R, int G, int B);
void setLedRGBI(int id, int R, int G, int B);
void setServo(int id, int value);
void shakeServo(int id, int n);

void setValue(int varId, int value);
void setValueI(int varId, int value);
int getValue(int varId);

#endif
