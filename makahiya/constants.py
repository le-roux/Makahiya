# This file contains all the codes to send to the plant.
# Don't modify it without also modifying it's equivalent on the plant-side.

class constants:

    LED1_R = 33
    LED1_G = 10
    LED1_B = 11
    LED1_ON = 50

    LED2_R = 34
    LED2_G = 6
    LED2_B = 7
    LED2_ON = 51

    LED3_R = 35
    LED3_G = 29
    LED3_B = 30
    LED3_ON = 52

    LED4_R = 31
    LED4_G = 0
    LED4_B = 36
    LED4_ON = 53

    LED5_R = 37
    LED5_G = 38
    LED5_B = 5
    LED5_ON = 54

    LED_HP_R = 8
    LED_HP_G = 9
    LED_HP_B = 12
    LED_HP_W = 13
    LED_HP_ON = 55

    LED_R = [LED_HP_R, LED1_R, LED2_R, LED3_R, LED4_R, LED5_R]
    LED_G = [LED_HP_G, LED1_G, LED2_G, LED3_G, LED4_G, LED5_G]
    LED_B = [LED_HP_B, LED1_B, LED2_B, LED3_B, LED4_B, LED5_B]
    LED_ON = [LED_HP_ON, LED1_ON, LED2_ON, LED3_ON, LED4_ON, LED5_ON]

    SERVO1 = 61
    SERVO2 = 62
    SERVO3 = 63
    SERVO4 = 64
    SERVO5 = 65

    SERVOS = [SERVO1, SERVO2, SERVO3, SERVO4, SERVO5]

    GET = 'get '
    SET = 'set '
