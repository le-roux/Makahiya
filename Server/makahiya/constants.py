# This file contains all the codes to send to the plant.
# Don't modify it without also modifying it's equivalent on the plant-side.

from .models import Leds

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

    LED_HP_R = 9
    LED_HP_G = 8
    LED_HP_B = 13
    LED_HP_W = 12
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
    MUSIC = '1 '


    SET_LED_ON = []
    SET_LED_OFF = []
    ALL_ON = ''
    ALL_OFF = ''

    for i in range(6):
        SET_LED_ON.append(' ' + str(LED_ON[i]) + ' ' + str(1))
        SET_LED_OFF.append(' ' + str(LED_ON[i]) + ' ' + str(0))
        ALL_ON += SET_LED_ON[i]
        ALL_OFF += SET_LED_OFF[i]

    ledHP_R_and_sound_1 = ' ' + str(11) + \
    ' ' + MUSIC + str(1) + \
    ' ' + str(LED_HP_R) + ' ' + str(150) + \
    ALL_OFF + \
    ' ' + str(LED_HP_G) + ' ' + str(0) + \
    ' ' + str(LED_HP_B) + ' ' + str(0) + \
    ' ' + str(LED_HP_W) + ' ' + str(0)

    LIGHT_CONFIG = ['1', 'Led 1 Red', 'Led 1 Green', 'Full Red', 'Full Green', 'Full Blue', 'Full White']

    FULL_RED = ''
    for i in range(6):
        FULL_RED += ' ' + str(LED_R[i]) + ' ' + str(150) + \
                ' ' + str(LED_G[i]) + ' ' + str(0) + \
                ' ' + str(LED_B[i]) + ' ' + str(0)
    FULL_RED += ' ' + str(LED_HP_W) + ' ' + str(0)
    for i in range(6):
        FULL_RED += ' ' + str(LED_ON[i]) + ' ' + str(1)

    FULL_GREEN = ''
    for i in range(6):
        FULL_GREEN += ' ' + str(LED_R[i]) + ' ' + str(0) + \
                ' ' + str(LED_G[i]) + ' ' + str(150) + \
                ' ' + str(LED_B[i]) + ' ' + str(0)
    FULL_GREEN += ' ' + str(LED_HP_W) + ' ' + str(0)
    for i in range(6):
        FULL_GREEN += ' ' + str(LED_ON[i]) + ' ' + str(1)

    FULL_BLUE = ''
    for i in range(6):
        FULL_BLUE += ' ' + str(LED_R[i]) + ' ' + str(0) + \
                ' ' + str(LED_G[i]) + ' ' + str(0) + \
                ' ' + str(LED_B[i]) + ' ' + str(150)
    FULL_BLUE += ' ' + str(LED_HP_W) + ' ' + str(0)
    for i in range(6):
        FULL_BLUE += ' ' + str(LED_ON[i]) + ' ' + str(1)

    FULL_WHITE = ''
    for i in range(6):
        FULL_WHITE += ' ' + str(LED_R[i]) + ' ' + str(150) + \
                ' ' + str(LED_G[i]) + ' ' + str(150) + \
                ' ' + str(LED_B[i]) + ' ' + str(150)
    FULL_WHITE += ' ' + str(LED_HP_W) + ' ' + str(150)
    for i in range(6):
        FULL_WHITE += ' ' + str(LED_ON[i]) + ' ' + str(1)

    FULL_YELLOW = ''
    for i in range(6):
        FULL_YELLOW += ' ' + str(LED_R[i]) + ' ' + str(150) + \
                ' ' + str(LED_G[i]) + ' ' + str(150) + \
                ' ' + str(LED_B[i]) + ' ' + str(0)
    FULL_YELLOW += ' ' + str(LED_HP_W) + ' ' + str(0)
    for i in range(6):
        FULL_YELLOW += ' ' + str(LED_ON[i]) + ' ' + str(1)

    FULL_PINK = ''
    for i in range(6):
        FULL_PINK += ' ' + str(LED_R[i]) + ' ' + str(150) + \
                ' ' + str(LED_G[i]) + ' ' + str(0) + \
                ' ' + str(LED_B[i]) + ' ' + str(150)
    FULL_PINK += ' ' + str(LED_HP_W) + ' ' + str(0)
    for i in range(6):
        FULL_PINK += ' ' + str(LED_ON[i]) + ' ' + str(1)

    FULL_CYAN = ''
    for i in range(6):
        FULL_CYAN += ' ' + str(LED_R[i]) + ' ' + str(0) + \
                ' ' + str(LED_G[i]) + ' ' + str(150) + \
                ' ' + str(LED_B[i]) + ' ' + str(150)
    FULL_CYAN += ' ' + str(LED_HP_W) + ' ' + str(0)
    for i in range(6):
        FULL_CYAN += ' ' + str(LED_ON[i]) + ' ' + str(1)

    ALL_OFF = ''
    for i in range(6):
        ALL_OFF += ' ' + str(LED_ON[i]) + ' ' + str(0)

def setAllLeds(SQLsession, plant_id, R, G, B, W):
    msg = ''
    for i in range(6):
        led = SQLsession.query(Leds).filter_by(plant_id=plant_id, led_id=i).first()
        led.R = R
        led.G = G
        led.B = B
        led.on = True
        msg += constants.SET + str(constants.LED_R[i]) + ' ' + str(R) + ' '
        msg += constants.SET + str(constants.LED_G[i]) + ' ' + str(G) + ' '
        msg += constants.SET + str(constants.LED_B[i]) + ' ' + str(B) + ' '
        msg += constants.SET + str(constants.LED_ON[i]) + ' ' + str(1) + ' '
        if i == 0:
            led.W = W
            msg = constants.SET + str(constants.LED_HP_W) + ' ' + str(W) + ' '
        SQLsession.commit()
    return msg

def allLedsOff(SQLsession, plant_id):
    msg = ''
    for i in range(6):
        led = SQLsession.query(Leds).filter_by(plant_id=plant_id, led_id=i).first()
        led.on = False
        msg += constants.SET + str(constants.LED_ON[i]) + ' ' + str(0) + ' '
        SQLsession.commit()
    return msg
