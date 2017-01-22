#include "ext_user.h"
#include "utils.h"
#include "fdc2214.h"
#include "websocket.h"

static void fdc_cb(EXTDriver* driver, expchannel_t channel);

EXTDriver* const EXTD = &EXTD1;

const EXTConfig ext_config = {
	{
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
		{EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOC, fdc_cb},
		{EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, web_cb},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOC, fdc_cb},
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

static void fdc_cb (EXTDriver* driver, expchannel_t channel) {
	UNUSED(driver);
	UNUSED(channel);
	chSysLockFromISR();
	if (channel == 10)
		calling = 1;
	else
		calling = 2;
	chBSemSignalI(&fdc_bsem);
	chSysUnlockFromISR();
}
