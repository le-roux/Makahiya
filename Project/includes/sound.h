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

extern uint32_t* buffer;

#define ALARM_SOUND_NB 3

extern int16_t _binary_alarm1_mp3_start;
extern int16_t _binary_alarm1_mp3_size;
extern int16_t _binary_alarm1_mp3_end;

extern int16_t _binary_alarm2_mp3_start;
extern int16_t _binary_alarm2_mp3_size;
extern int16_t _binary_alarm2_mp3_end;

extern int16_t _binary_alarm3_mp3_start;
extern int16_t _binary_alarm3_mp3_size;
extern int16_t _binary_alarm3_mp3_end;

extern const int16_t* const _binary_start[ALARM_SOUND_NB];
extern const int16_t* const _binary_size[ALARM_SOUND_NB];
extern const int16_t* const _binary_end[ALARM_SOUND_NB];

extern volatile int music_id;
extern volatile int repeat;

#define FLASH_SECTOR_SIZE 131072 // 128k
#define FLASH_SECTOR_NB 6
/**
 * Size of a sample in bytes
 */
#define SAMPLE_SIZE 2

/**
 * Size of buffers that hold mp3 decoded frames (in number of samples)
 */
#define I2S_BUF_SIZE MAX_NSAMP * MAX_NGRAN * MAX_NCHAN

/**
 * The buffer used for DMA transfer.
 * Each half of this buffer holds a complete mp3 decoded frame.
 */
extern int16_t i2s_tx_buf[I2S_BUF_SIZE * 2];

/**
 * Number of intermediate buffers available for the mailbox
 */
#define AUDIO_BUFFERS_NB 2

/**
 * Multiplier for the volume
 * Formula: sample * volumeMult / volumeDiv
 */
extern int8_t volumeMult;

/**
 * Divider for the volume
 * Formula: sample * volumeMult / volumeDiv
 */
extern int8_t volumeDiv;

#define WORKING_BUFFER_SIZE 4000

#define INPUT_BUFFER_SIZE 1000
#define INPUT_BUFFERS_NB 5

#define I2SDIV 6

extern I2SConfig audio_i2s_cfg;

extern THD_WORKING_AREA(wa_audio, 1024);

extern THD_WORKING_AREA(wa_audio_in, 2048);
extern THD_WORKING_AREA(wa_flash, 2048);
extern binary_semaphore_t audio_bsem;
extern binary_semaphore_t download_bsem;

/******************************/
/*        Functions           */
/******************************/
void sound_set_pins(void);
void i2s_cb(I2SDriver* driver, size_t offset, size_t n);

extern THD_FUNCTION(audio_playback, arg);
extern THD_FUNCTION(wifi_audio_in, arg);
extern THD_FUNCTION(flash_audio_in, arg);

#endif // SOUND_H
