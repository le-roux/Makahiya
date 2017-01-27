#include "ch.h"
#include "hal.h"
#include <math.h>
#include "utils.h"
#include <string.h>
#include "chprintf.h"
#include "RTT_streams.h"

#include "sound.h"
#include "wifi.h"

#include "flash.h"

const int16_t* const _binary_start[ALARM_SOUND_NB] = {&_binary_alarm1_mp3_start,
	&_binary_alarm2_mp3_start,
	&_binary_alarm3_mp3_start};

const int16_t* const _binary_size[ALARM_SOUND_NB] = {&_binary_alarm1_mp3_size,
	&_binary_alarm2_mp3_size,
	&_binary_alarm3_mp3_size};

const int16_t* const _binary_end[ALARM_SOUND_NB] = {&_binary_alarm1_mp3_end,
	&_binary_alarm2_mp3_end,
	&_binary_alarm3_mp3_end};
volatile int music_id;
static volatile int music_source;
#define DOWNLOAD 0
#define INNER 1
volatile int repeat;

int16_t i2s_tx_buf[I2S_BUF_SIZE * 2];
thread_reference_t audio_thread_ref = NULL;

/**
 * Buffers used to store decoded data before sending them through I2S.
 */
static int16_t buf[AUDIO_BUFFERS_NB][I2S_BUF_SIZE];
static msg_t audio_buffers[AUDIO_BUFFERS_NB];
static msg_t free_audio_buffers[AUDIO_BUFFERS_NB];
MAILBOX_DECL(audio_box, audio_buffers, AUDIO_BUFFERS_NB);
MAILBOX_DECL(free_box, free_audio_buffers, AUDIO_BUFFERS_NB);

/**
 * Buffers used to store mp3 data when copied from flash and sent to decoder.
 */
static int16_t in_buf[INPUT_BUFFERS_NB][INPUT_BUFFER_SIZE];
static msg_t input_buffers[INPUT_BUFFERS_NB];
static msg_t free_input_buffers[INPUT_BUFFERS_NB];
MAILBOX_DECL(input_box, input_buffers, INPUT_BUFFERS_NB);
MAILBOX_DECL(free_input_box, free_input_buffers, INPUT_BUFFERS_NB);

/**
 * Buffers used when downloading music (flash memory).
 */
static int8_t flash_sec[FLASH_SECTOR_NB][FLASH_SECTOR_SIZE] __attribute__((section(".flashdata")));
static msg_t flash_sectors[FLASH_SECTOR_NB];
static msg_t free_flash_sectors[FLASH_SECTOR_NB];
MAILBOX_DECL(flash_box, flash_sectors, FLASH_SECTOR_NB);
MAILBOX_DECL(free_flash_box, free_flash_sectors, FLASH_SECTOR_NB);

static SEMAPHORE_DECL(audio_sem, 2);
BSEMAPHORE_DECL(audio_bsem, true);
BSEMAPHORE_DECL(decode_bsem, true);
BSEMAPHORE_DECL(download_bsem, true);

static int8_t working_buffer[WORKING_BUFFER_SIZE];

volatile int count = 0;
volatile bool started = false;

int8_t volumeMult = 1;
int8_t volumeDiv = 1;

I2SConfig audio_i2s_cfg = {
	i2s_tx_buf,
	NULL,
	I2S_BUF_SIZE * 2,
	i2s_cb,
	0,
	SPI_I2SPR_MCKOE | I2SDIV
};

void i2s_cb(I2SDriver* driver, size_t offset, size_t n) {
	UNUSED(driver);
	UNUSED(offset);
	UNUSED(n);
	// Signal that a buffer is free.
	chSysLockFromISR();
	chSemSignalI(&audio_sem);
	chSysUnlockFromISR();
}

THD_WORKING_AREA(wa_audio, 1024);

THD_FUNCTION(audio_playback, arg) {
	int bytes_left = 0, count = 0, bytes_consumed = FLASH_SECTOR_SIZE, copy;
	bool ended;
	HMP3Decoder decoder;
	unsigned char* read_ptr;
	volatile int offset, err;
	void* pbuf, *inbuf;
	UNUSED(arg);

	// Init the free buffers mailbox
	for (int i = 0; i < AUDIO_BUFFERS_NB; i++)
		(void)chMBPost(&free_box, (msg_t)&buf[i], TIME_INFINITE);

	decoder = MP3InitDecoder();

	while(TRUE) {
		ended = false;
		chBSemWait(&decode_bsem);
		while (TRUE) {
			// Get a free buffer (for output)
			if (chMBFetch(&free_box, (msg_t*)&pbuf, TIME_INFINITE) != MSG_OK)
				break; // Error that can be corrected.

			// Acquire new data if possible
			if (music_source == INNER) {
				while (WORKING_BUFFER_SIZE - bytes_left >= 2 * INPUT_BUFFER_SIZE) { // Space available for new data
					// Get an input buffer
					chMBFetch(&input_box, (msg_t*)&inbuf, TIME_INFINITE);
					if (inbuf == NULL) { // End of music
						chMBPost(&free_box, (msg_t)pbuf, TIME_INFINITE);
						chMBPost(&free_input_box, (msg_t)inbuf, TIME_INFINITE);
						if (I2SD2.state != I2S_STOP) {
							i2sStopExchange(&I2SD2);
							i2sStop(&I2SD2);
						}
						chSemReset(&audio_sem, 2);
						started = false;
						ended = true;
						break;
					}

					// Copy the new data at the end of the working buffer
					memcpy(&working_buffer[bytes_left], inbuf, INPUT_BUFFER_SIZE * 2);
					bytes_left += INPUT_BUFFER_SIZE * 2;

					// Release the input buffer
					chMBPost(&free_input_box, (msg_t)inbuf, TIME_INFINITE);
				}
			} else { // Music source is download.
				if (bytes_consumed == FLASH_SECTOR_SIZE) { // Need to get a new flash sector.
					chMBFetch(&flash_box, (msg_t*)&inbuf, TIME_INFINITE);
					bytes_consumed = 0;
					if (inbuf == NULL) { // End of music
						chMBPost(&free_box, (msg_t)pbuf, TIME_INFINITE);
						chMBPost(&free_input_box, (msg_t)inbuf, TIME_INFINITE);
						if (I2SD2.state != I2S_STOP) {
							i2sStopExchange(&I2SD2);
							i2sStop(&I2SD2);
						}
						chSemReset(&audio_sem, 2);
						started = false;
						ended = true;
						break;
					}
				}
				copy = WORKING_BUFFER_SIZE - bytes_left;
				if (copy > 1000) {
					if (copy > FLASH_SECTOR_SIZE - bytes_consumed)
						copy = FLASH_SECTOR_SIZE - bytes_consumed;
					memcpy(&working_buffer[bytes_left], (int8_t*)inbuf + bytes_consumed, copy);
					bytes_left += copy;
					bytes_consumed += copy;
					if (bytes_consumed == FLASH_SECTOR_SIZE) {
						chMBPost(&free_flash_box, (msg_t)inbuf, TIME_INFINITE);
					}
				}
			}
			if (ended)
				break;

			read_ptr = (unsigned char*)working_buffer;
			offset = MP3FindSyncWord(read_ptr, bytes_left);
			bytes_left -=  offset;
			memmove(working_buffer, working_buffer + offset, bytes_left);

			// Decode the mp3 block
			err = MP3Decode(decoder, &read_ptr, &bytes_left, (int16_t*)pbuf, 0);
			memmove(working_buffer, read_ptr, bytes_left);

			if (err == -6) { // TODO: take care of other errors
				DEBUG("err %i", err);
				chMBPost(&free_box, (msg_t)pbuf, TIME_INFINITE);
				if (I2SD2.state != I2S_STOP) {
					chThdSleepMilliseconds(100);
					i2sStopExchange(&I2SD2);
					i2sStop(&I2SD2);
					started = false;
				}
				// Clear working buffer
				bytes_left = 0;
				read_ptr = (unsigned char*)working_buffer;
				// Restart
				continue;
			} else if (err == -1 || err == -9) {
				chMBPost(&free_box, (msg_t)pbuf, TIME_INFINITE);
				continue;
			}

			// Post the filled buffer
			count++;

			if (count % 2 == 0)
				offset = 0;
			else
				offset = I2S_BUF_SIZE;

			// Wait for free space in the i2s buffer.
			chSemWait(&audio_sem);
			// Copy the data in the i2s buffer.
			for (int i = 0; i < I2S_BUF_SIZE; i++)
				i2s_tx_buf[offset + i] = ((int16_t*)pbuf)[i] * volumeMult / volumeDiv;

			// Release the intermediate buffer.
			chMBPost(&free_box, (msg_t)pbuf, TIME_INFINITE);

			// Start the i2s if it's time.
			chSysLock();
			if (!started && chSemGetCounterI(&audio_sem) == 0) {
				chSysUnlock();
				i2sStart(&I2SD2, &audio_i2s_cfg);
				i2sStartExchange(&I2SD2);
				started = true;
			} else {
				chSysUnlock();
			}
		}
	}
}

THD_WORKING_AREA(wa_audio_in, 2048);
// To use for downloaded music.
THD_FUNCTION(wifi_audio_in, arg) {
	UNUSED(arg);

	/**
	 * Header of the WiFi answers.
	 */
	static wifi_response_header out;

	/**
	 * Number of bytes read (for the current buffer).
	 */
	static int bytes_nb;

	/**
	 * Number of bytes already read in the current frame.
	 */
	static int bytes_consumed;

	/**
	 * Number of bytes to copy from the input frame to the internal buffer.
	 */
	static int copy_size;

	/**
	 * Current buffer id (every buffer corresponds to a 128k flash sector).
	 */
	static flashsector_t buffer_id;

	/**
	 * Boolean value indicating that the WiFi module can't provide more data
	 * for the moment.
	 */
	static int end;

	/**
	 * Pointer to buffers used in the mailbox.
	 */
	static void* inbuf;

	static int initial_flash_sector;

	static int initial_buffering;
	initial_flash_sector = flashSectorAt((flashaddr_t)&flash_sec[0]);

	// Init the free input buffers mailbox
	chSysLock();
	for (int i = initial_flash_sector; i < initial_flash_sector + FLASH_SECTOR_NB; i++) {
		chMBPostI(&free_flash_box, (msg_t)&flash_sec[i - initial_flash_sector]);
		flashSectorErase(i);
	}
	chSysUnlock();

	while (TRUE) {
		end = 0;
		chBSemWait(&download_bsem);
		music_source = DOWNLOAD;
		initial_buffering = 0;
		while (end != 2) {
			chMBFetch(&free_flash_box, (msg_t*)&inbuf, TIME_INFINITE);
			buffer_id = flashSectorAt((flashaddr_t)inbuf);
			// Clear the flashdata section.
			if (initial_buffering >= FLASH_SECTOR_NB) {
				chSysLock();
				flashSectorErase(buffer_id);
				chSysUnlock();
			}

			// Read file from wifi
			bytes_nb = 0;
			bytes_consumed = WIFI_BUFFER_SIZE;
			out.length = WIFI_BUFFER_SIZE;
			out.error = 0;
			out.error_code = 0;

			while(bytes_nb < FLASH_SECTOR_SIZE) {
				while (bytes_consumed >= out.length) { // Need to perform a new read.
					bytes_consumed = 0;
					read_buffer(audio_conn);
					out = get_response(true);
					if (out.error && out.error_code == NO_DATA) {
						if (end) {
							end = 2;
							break;
						} else {
							end = 1;
							chThdSleepMilliseconds(1000);
						}
					} else
						end = 0;
				}
				if (end == 2)
					break;

				/**
				 * copy = min(out.length - bytes_consumed,
				 *            MUSIC_BUFFER_SIZE - bytes_nb)
				 */
				copy_size = out.length - bytes_consumed;
				if (FLASH_SECTOR_SIZE - bytes_nb < copy_size)
					copy_size = FLASH_SECTOR_SIZE - bytes_nb;

				// Don't copy more than remaining space.
				chSysLock();
				flashWrite((flashaddr_t)((int8_t*)flashSectorBegin(buffer_id) + bytes_nb),
						&response_body[bytes_consumed], copy_size);
				chSysUnlock();
				bytes_nb += copy_size;
				bytes_consumed += copy_size;
			}
			chMBPost(&flash_box, (msg_t)inbuf, TIME_INFINITE);
			initial_buffering++;
			if (initial_buffering > 2)
				chBSemSignal(&decode_bsem);
		}
		chMBFetch(&free_flash_box, (msg_t*)&inbuf, TIME_INFINITE);
		inbuf = NULL;
		chMBPost(&flash_box, (msg_t)inbuf, TIME_INFINITE);
	}
}

THD_WORKING_AREA(wa_flash, 2048);
// To use for alarm clock
THD_FUNCTION(flash_audio_in, arg) {
	UNUSED(arg);

	/**
	 * Pointer to the position of the next data to read (in music_buffer).
	 */
	static const int16_t* cur_pos;

	/**
	 * Pointer to buffers used in the mailbox.
	 */
	static void* inbuf;

	/**
	 * Id of the 'inner' music currently played.
	 */
	static int cur_id;

	// Init the free input buffers mailbox
	for (int i = 0; i < INPUT_BUFFERS_NB; i++)
		chMBPost(&free_input_box, (msg_t)&in_buf[i], TIME_INFINITE);

	while(TRUE) {
		chBSemWait(&audio_bsem);
		// Start to play music.
		music_source = INNER;
		cur_id = music_id;
		chBSemSignal(&decode_bsem);
		do {
			cur_pos = _binary_start[cur_id];
			while (cur_pos + INPUT_BUFFER_SIZE < _binary_end[cur_id]) {
				chMBFetch(&free_input_box, (msg_t*)&inbuf, TIME_INFINITE);
				memcpy(inbuf, cur_pos, 2 * INPUT_BUFFER_SIZE);
				cur_pos += INPUT_BUFFER_SIZE;
				chMBPost(&input_box, (msg_t)inbuf, TIME_INFINITE);
				if (!repeat)
					break;
			}
		} while(repeat);
		chMBFetch(&free_input_box, (msg_t*)&inbuf, TIME_INFINITE);
		inbuf = NULL;
		chMBPost(&input_box, (msg_t)inbuf, TIME_INFINITE);
	}
}

void init_audio(void){
	palSetPadMode(GPIOB, 12,  PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOB, 13,  PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOB, 15,  PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOC, 6,  PAL_MODE_ALTERNATE(5));
}
