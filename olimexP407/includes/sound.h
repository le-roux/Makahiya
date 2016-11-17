#ifndef SOUND_H
#define SOUND_H

/******************************/
/*       Variables            */
/******************************/

#ifndef M_PI
#define M_PI 3.1415926535
#endif // M_PI

extern int16_t _binary_pic_pcm_start;
extern int16_t _binary_pic_pcm_size;

#define I2S_BUF_SIZE 2048
static uint32_t i2s_tx_buf[I2S_BUF_SIZE];
static uint32_t sound_index;

#define I2SDIV 6

extern const I2SConfig i2s3_cfg;

/******************************/
/*        Functions           */
/******************************/
void sound_set_pins(void);
void sound_init(void);
void sound_440(void);
void i2s_cb(I2SDriver* driver, size_t offset, size_t n);

#endif // SOUND_H
