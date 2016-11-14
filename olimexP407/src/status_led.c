#include "status_led.h"
#include "pwm_user.h"

int brightness_1, brightness_2;

int LED_ID(int id) {
    switch(id) {
        case(0) : return STAT(1);
        case(1) : return STAT(2);
        case(2) : return STAT(3);
        case(3) : return STAT(4);
    }
    return -1;
}

void update_stat_led(void) {
    pwmEnableChannel(&PWMD10, 0, brightness_1);
    pwmEnableChannel(&PWMD11, 0, brightness_2);
    pwmEnableChannel(&PWMD13, 0, brightness_2);
    pwmEnableChannel(&PWMD14, 0, brightness_1);
}

void update_stat_ledI(void) {
    pwmEnableChannelI(&PWMD10, 0, brightness_1);
    pwmEnableChannelI(&PWMD11, 0, brightness_2);
    pwmEnableChannelI(&PWMD13, 0, brightness_2);
    pwmEnableChannelI(&PWMD14, 0, brightness_1);
}
