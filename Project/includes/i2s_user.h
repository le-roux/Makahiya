#ifndef I2S_USER_H
#define I2S_USER_H

#include "../helix/pub/mp3dec.h"
#include "hal.h"

/************************/
/*        Defines       */
/************************/

/**
 * Size of buffers that hold mp3 decoded frames (in number of samples)
 */
#define I2S_BUF_SIZE MAX_NSAMP * MAX_NGRAN * MAX_NCHAN

/************************/
/*       Variables      */
/************************/

/**
 * The buffer used for DMA transfer.
 * Each half of this buffer holds a complete mp3 decoded frame.
 */
extern int16_t i2s_tx_buf[I2S_BUF_SIZE * 2];

extern I2SConfig audio_i2s_cfg;

#endif // I2S_USER_H
