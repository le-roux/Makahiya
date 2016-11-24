#ifndef SOUND_H
#define SOUND_H
/******************************/
/*        Includes            */
/******************************/

// Includes for Helix code
#include "../helix/pub/mp3dec.h"

/******************************/
/*       Variables            */
/******************************/

#ifndef M_PI
#define M_PI 3.1415926535
#endif // M_PI

extern uint32_t* buffer;

extern int16_t _binary_pic_mp3_start;
extern int16_t _binary_pic_mp3_size;
extern int16_t _binary_pic_mp3_end;

#define I2S_BUF_SIZE MAX_NSAMP*2
#define I2S_HALF_BUF_SIZE MAX_NSAMP
extern uint32_t i2s_tx_buf[I2S_BUF_SIZE];
extern binary_semaphore_t audio_sem;

#define I2SDIV 6

extern I2SConfig i2s3_cfg;

extern THD_WORKING_AREA(wa_audio, 4096);

/******************************/
/*        Functions           */
/******************************/
void sound_set_pins(void);
void sound_init(void);
void sound_440(void);
void i2s_cb(I2SDriver* driver, size_t offset, size_t n);

extern THD_FUNCTION(audio_playback, arg);

#endif // SOUND_H
