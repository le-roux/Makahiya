#ifndef SOUND_H
#define SOUND_H

#ifndef M_PI
#define M_PI 3.1415926535
#endif // M_PI

void sound_init(void);
void sound_440(void);
// TODO: clean all the sound related code
extern int16_t _binary_pic_pcm_start;
extern int16_t _binary_pic_pcm_size;

#define I2S_BUF_SIZE 4096
static uint16_t i2s_tx_buf[I2S_BUF_SIZE];
static uint32_t sound_index;

#define I2SDIV 6
#define ODD 1 << 8
#define MCKOE 1 << 9

void i2s_cb(I2SDriver* driver, size_t offset, size_t n);

const I2SConfig i2s3_cfg = {
    i2s_tx_buf,
    NULL,
    I2S_BUF_SIZE,
    i2s_cb,
    0x0038,
    MCKOE | I2SDIV | ODD
};

#endif // SOUND_H
