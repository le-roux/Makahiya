#include "ext_user.h"
#include "status_led.h"
#include "utils.h"
#include "pwm_user.h"

const EXTConfig ext_config = {
    {
        {EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, ext_cb1},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOC, ext_cb2},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL}
    }
};

static int direction_1, direction_2;
static volatile bool irq_suspended_1, irq_suspended_2;
static virtual_timer_t irq_vt;

void ext_init(void) {
    brightness_1 = 0;
    direction_1 = 0;
    brightness_2 = 0;
    direction_2 = 0;

    irq_suspended_1 = FALSE;
    irq_suspended_2 = FALSE;

    chVTObjectInit(&irq_vt);
    extStart(&EXTD1, &ext_config);
}

void irq_cb(void* param);

/**
 * Increase or decrease the brightness of the STAT1 led when the WAKEUP button is
 * clicked.
 */
void ext_cb1(EXTDriver* driver, expchannel_t channel) {
    UNUSED(driver);
    UNUSED(channel);
    if (!irq_suspended_1) {
        irq_suspended_1 = TRUE;
        if (brightness_1 < 100)
            direction_1 = 0;
        else if (brightness_1 > 900)
            direction_1 = 1;
        if (direction_1 == 0)
            brightness_1 += 100;
        else
             brightness_1 -= 100;
        chSysLockFromISR();
        pwmEnableChannelI(&PWMD10, 0, brightness_1);
        chVTSetI(&irq_vt, MS2ST(SUSPEND_TIME), irq_cb, (void*)1);
        chSysUnlockFromISR();
    }
}

/**
 * Increase or decrease the brightness of the STAT2 led when the TAMPER button is
 * clicked.
 */
void ext_cb2(EXTDriver* driver, expchannel_t channel) {
    UNUSED(driver);
    UNUSED(channel);
    if (!irq_suspended_2) {
        irq_suspended_2 = TRUE;
        if (brightness_2 < 100)
            direction_2 = 0;
        else if (brightness_2 > 900)
            direction_2 = 1;
        if (direction_2 == 0)
            brightness_2 += 100;
        else
             brightness_2 -= 100;
        chSysLockFromISR();
        pwmEnableChannelI(&PWMD11, 0, brightness_2);
        chVTSetI(&irq_vt, MS2ST(SUSPEND_TIME), irq_cb, (void*)2);
        chSysUnlockFromISR();
    }
}

/**
 * Activate the proper ext_cb.
 * @param param: The id of the callback to activate (1 or 2).
 */
void irq_cb(void* param) {
    if ((int)param == 1)
        irq_suspended_1 = FALSE;
    else
        irq_suspended_2 = FALSE;
}
