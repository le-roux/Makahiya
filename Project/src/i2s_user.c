#include "i2s_user.h"
#include "sound.h"

int16_t i2s_tx_buf[I2S_BUF_SIZE * 2];

I2SConfig audio_i2s_cfg = {
	i2s_tx_buf,
	NULL,
	I2S_BUF_SIZE * 2,
	audioI2Scb,
	0,
	SPI_I2SPR_MCKOE | I2SDIV
};
