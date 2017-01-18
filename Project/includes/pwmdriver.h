#ifndef PWM_H
#define PWM_H

#define LED1_R 33U
#define LED1_G 10U
#define LED1_B 11U

#define LED2_R 34U
#define LED2_G 6U
#define LED2_B 7U

#define LED3_R 35U
#define LED3_G 29U
#define LED3_B 30U

#define LED4_R 31U
#define LED4_G 0U
#define LED4_B 36U

#define LED5_R 37U
#define LED5_G 38U
#define LED5_B 5U

#define LED_HP_R 8U
#define LED_HP_G 9U
#define LED_HP_B 12U
#define LED_HP_W 13U

void initPwm(void);
void setLed(int led, int power);

void setLedHP(int R, int G, int B, int W);
void setLedRGB(int id, int R, int G, int B);
void setServo(int id, int value);

#endif
