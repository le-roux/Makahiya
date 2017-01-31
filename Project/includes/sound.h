#ifndef SOUND_H
#define SOUND_H
/******************************/
/*        Includes            */
/******************************/

// Includes for Helix code
#include "../helix/pub/mp3dec.h"

#include "wifi.h"

/******************************/
/*       Variables            */
/******************************/

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

/**
 * Id of the music stored in flash to play.
 * This variable must be set before signaling the @p audio_bsem binary semaphore.
 */
extern volatile int music_id;

/**
 * Value indicating when the alarm music must be stopped. Set it to 1 before
 * starting the music.
 */
extern volatile int repeat;

/**
 * Variable to store the channel id of the audio download connection.
 */
extern volatile wifi_connection audio_conn;

extern binary_semaphore_t audio_bsem;
extern binary_semaphore_t download_bsem;

extern THD_WORKING_AREA(wa_audio, 512);

extern THD_WORKING_AREA(wa_audio_in, 128);
extern THD_WORKING_AREA(wa_flash, 128);

/******************************/
/*        Functions           */
/******************************/

/**
 * @brief Initialize all the elements related to the audio.
 */
void audioInit(void);

/**
 * @brief Function called by the I2S driver when a transfer is finished.
 */
void audioI2Scb(I2SDriver* driver, size_t offset, size_t n);

extern THD_FUNCTION(audio_playback, arg);
extern THD_FUNCTION(wifi_audio_in, arg);
extern THD_FUNCTION(flash_audio_in, arg);

#endif // SOUND_H
