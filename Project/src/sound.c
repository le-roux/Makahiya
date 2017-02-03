#include "ch.h"
#include "hal.h"
#include <math.h>
#include "utils.h"
#include <string.h>
#include "chprintf.h"
#include "RTT_streams.h"
#include "i2s_user.h"

#include "sound.h"
#include "wifi.h"

#include "flash.h"

const int8_t* const _binary_start[ALARM_SOUND_NB] = {&_binary_alarm1_mp3_start,
	&_binary_alarm2_mp3_start,
	&_binary_alarm3_mp3_start,
	&_binary_boot_mp3_start};

const int8_t* const _binary_size[ALARM_SOUND_NB] = {&_binary_alarm1_mp3_size,
	&_binary_alarm2_mp3_size,
	&_binary_alarm3_mp3_size,
	&_binary_boot_mp3_size};

const int8_t* const _binary_end[ALARM_SOUND_NB] = {&_binary_alarm1_mp3_end,
	&_binary_alarm2_mp3_end,
	&_binary_alarm3_mp3_end,
	&_binary_boot_mp3_end};


volatile int music_id;
volatile bool repeat;
volatile wifi_connection audio_conn;
static MUTEX_DECL(audio_mutex);

/**
 * Number of buffers handled by the @p audio_box and @p free_box mailboxes.
 */
#define DECODED_DATA_BUFFERS_NB 2

static const int WORKING_BUFFER_MIN_LENGTH = 500;

/**
 * Buffers used to store decoded data before sending them through I2S.
 */
static int16_t buf[DECODED_DATA_BUFFERS_NB][I2S_BUF_SIZE];
static msg_t decoded_data_buffers[DECODED_DATA_BUFFERS_NB];
static msg_t free_decoded_data_buffers[DECODED_DATA_BUFFERS_NB];
MAILBOX_DECL(decoded_data_box, decoded_data_buffers, DECODED_DATA_BUFFERS_NB);
MAILBOX_DECL(free_decoded_data_box, free_decoded_data_buffers, DECODED_DATA_BUFFERS_NB);

/**
 * Number of buffers handled by the @p input_box and @p free_input_box mailboxes.
 */
#define INPUT_BUFFERS_NB 15

/**
 * Size of the buffers handled by @p input_box and @p free_input_box.
 */
#define INPUT_BUFFER_SIZE 2000

/**
 * Buffers used to store mp3 data when copied from flash and sent to decoder.
 */
static int8_t in_buf[INPUT_BUFFERS_NB][INPUT_BUFFER_SIZE];
static msg_t input_buffers[INPUT_BUFFERS_NB];
static msg_t free_input_buffers[INPUT_BUFFERS_NB];
MAILBOX_DECL(input_box, input_buffers, INPUT_BUFFERS_NB);
MAILBOX_DECL(free_input_box, free_input_buffers, INPUT_BUFFERS_NB);

static SEMAPHORE_DECL(audio_sem, 2);
BSEMAPHORE_DECL(audio_bsem, true);
BSEMAPHORE_DECL(decode_bsem, true);
BSEMAPHORE_DECL(download_bsem, true);

#define WORKING_BUFFER_SIZE 5000

static int8_t working_buffer[WORKING_BUFFER_SIZE];

static volatile bool started = false;

void reset_mailboxes(void) {
	chMBReset(&free_input_box);
	chMBReset(&input_box);
	chMBReset(&free_decoded_data_box);
	chMBReset(&decoded_data_box);

	// Init the free input buffers mailbox
	for (int i = 0; i < INPUT_BUFFERS_NB; i++)
		(void)chMBPost(&free_input_box, (msg_t)&in_buf[i], TIME_INFINITE);

	// Init the free buffers mailbox
	for (int i = 0; i < DECODED_DATA_BUFFERS_NB; i++)
		(void)chMBPost(&free_decoded_data_box, (msg_t)&buf[i], TIME_INFINITE);
}

void audioInit(void){
	palSetPadMode(GPIOB, 12,  PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOB, 13,  PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOB, 15,  PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOC, 6,  PAL_MODE_ALTERNATE(5));

	reset_mailboxes();
}

void audioI2Scb(I2SDriver* driver, size_t offset, size_t n) {
	UNUSED(driver);
	UNUSED(offset);
	UNUSED(n);
	// Signal that a buffer is free.
	chSysLockFromISR();
	chSemSignalI(&audio_sem);
	chSysUnlockFromISR();
}

THD_WORKING_AREA(wa_audio, 512);

THD_FUNCTION(audio_playback, arg) {

	static int bytes_left;
	/**
	 * Number of buffer posted to the i2s driver.
	 */
	int count = 0;

	/**
	 * Boolean value indicating if the current decoding session is finished.
	 */
	bool ended;

	/**
	 * Object used to perform the decoding.
	 */
	HMP3Decoder decoder;

	/**
	 * Pointer to the next interesting byte in the working buffer.
	 */
	int8_t* read_ptr;

	/**
	 * Pointer to the next byte to write in the working buffer.
	 */
	int8_t* write_ptr;

	/**
	 * Number of bytes in the working buffer before the next start of frame.
	 */
	int offset;

	/**
	 * Offset in the i2s buffer indicating where to write the next decoded data.
	 */
	int i2s_offset;

	/**
	 * Return value of the MP3Decode function.
	 */
	int err;

	/**
	 * Pointers to the input and output buffers.
	 */
	void* pbuf, *inbuf;

	/**
	 * Number of input buffers availablein the mailbox.
	 */
	int buf_nb;

	UNUSED(arg);

	decoder = MP3InitDecoder();

	while(true) {
		ended = false;
		read_ptr = working_buffer;
		write_ptr = working_buffer;
		// Wait to be triggered by one of the reading threads.
		chBSemWait(&decode_bsem);

		while (true) {
			// Get a free buffer (for output)
			chMBFetch(&free_decoded_data_box, (msg_t*)&pbuf, TIME_INFINITE);

			// Acquire new data if possible
			while (working_buffer + WORKING_BUFFER_SIZE - write_ptr >= INPUT_BUFFER_SIZE) { // While there is space available for new data
				chSysLock();
				buf_nb = chMBGetUsedCountI(&input_box);
				chSysUnlock();

				// Don't wait if there is no buffer available and still enough data.
				if (buf_nb == 0 && (write_ptr - read_ptr) > WORKING_BUFFER_MIN_LENGTH) {
					break;
				} else if (buf_nb == 0 && I2SD2.state != I2S_STOP) {
					i2sStopExchange(&I2SD2);
					i2sStop(&I2SD2);
					started = false;
				}

				// Get an input buffer
				chMBFetch(&input_box, (msg_t*)&inbuf, TIME_INFINITE);

				// End of music
				if (inbuf == NULL) {
					chMBPost(&free_decoded_data_box, (msg_t)pbuf, TIME_INFINITE);
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
				memcpy(write_ptr, inbuf, INPUT_BUFFER_SIZE);
				write_ptr += INPUT_BUFFER_SIZE;

				// Release the input buffer
				chMBPost(&free_input_box, (msg_t)inbuf, TIME_INFINITE);
			}
			if (ended)
				break;

			offset = MP3FindSyncWord((unsigned char*)read_ptr, write_ptr - read_ptr);

			if (offset < 0) { // No sync word in the working buffer: no more interesting data.
				offset = 0;
				read_ptr = working_buffer;
				write_ptr = working_buffer;
				continue;
			} else { // Normal behaviour.
				read_ptr +=  offset;
			}

			if (read_ptr - working_buffer > INPUT_BUFFER_SIZE) {
				memmove(working_buffer, read_ptr, write_ptr - read_ptr);
				write_ptr -= (read_ptr - working_buffer);
				read_ptr = working_buffer;
			}

			// Decode the mp3 block
			bytes_left = write_ptr - read_ptr;
			err = MP3Decode(decoder, (unsigned char**)&read_ptr, &bytes_left, (int16_t*)pbuf, 0);

			if (read_ptr - working_buffer > INPUT_BUFFER_SIZE) {
				memmove(working_buffer, read_ptr, write_ptr - read_ptr);
				write_ptr -= (read_ptr - working_buffer);
				read_ptr = working_buffer;
			}

			if (err == ERR_MP3_INVALID_FRAMEHEADER) {
				DEBUG("err %i", err);
				chMBPost(&free_decoded_data_box, (msg_t)pbuf, TIME_INFINITE);
				if (I2SD2.state != I2S_STOP) {
					chThdSleepMilliseconds(100);
					i2sStopExchange(&I2SD2);
					i2sStop(&I2SD2);
					started = false;
				}

				// Clear working buffer
				read_ptr = working_buffer;
				write_ptr = working_buffer;
				// Restart
				continue;
			} else if (err == ERR_MP3_INDATA_UNDERFLOW || err == ERR_MP3_INVALID_HUFFCODES) {
				chMBPost(&free_decoded_data_box, (msg_t)pbuf, TIME_INFINITE);
				continue;
			}

			count++;
			if (count % 2 == 0)
				i2s_offset = 0;
			else
				i2s_offset = I2S_BUF_SIZE;

			// Wait for free space in the i2s buffer.
			chSemWait(&audio_sem);

			// Copy the data in the i2s buffer.
			memcpy(&i2s_tx_buf[i2s_offset], pbuf, 2 * I2S_BUF_SIZE);

			// Release the intermediate buffer.
			chMBPost(&free_decoded_data_box, (msg_t)pbuf, TIME_INFINITE);

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

THD_WORKING_AREA(wa_audio_in, 128);
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
	 * Pointer to buffers used in the mailbox.
	 */
	static void* inbuf;

	/**
	 * Number of buffers to fill before starting the decoding thread.
	 */
	static int initial_buffering;

	/**
	 * Counter that indicates how many times a NO_DATA error has been returned
	 * consecutively.
	 */
	int count_nodata;

	int buf_nb;

	while (true) {
		// Wait to be triggered by the user.
		chBSemWait(&download_bsem);

		chMtxLock(&audio_mutex);
		reset_mailboxes();
		chMtxUnlock(&audio_mutex);

		chSysLock();
		buf_nb = chMBGetUsedCountI(&free_input_box);
		chSysUnlock();

		count_nodata = 0;
		initial_buffering = INPUT_BUFFERS_NB - 1;
		out.length = INPUT_BUFFER_SIZE;
		out.error = 0;
		out.error_code = NO_ERROR;
		while (count_nodata < 6 && repeat) {
			chMtxLock(&audio_mutex);
			chMBFetch(&free_input_box, (msg_t*)&inbuf, TIME_INFINITE);

			bytes_nb = 0;

			// Read file from wifi
			while(bytes_nb < INPUT_BUFFER_SIZE) { // while buffer is not full.
				out = wifi_read(audio_conn, &((uint8_t*)inbuf)[bytes_nb], INPUT_BUFFER_SIZE - bytes_nb, true, false);
				if (out.error && out.error_code == NO_DATA) {
					count_nodata++;
					if (count_nodata == 6)
						break;
					chThdSleepMilliseconds(1000);
				} else {
					count_nodata = 0;
				}
				bytes_nb += out.length;
			}

			// Post the filled buffer.
			chMBPost(&input_box, (msg_t)inbuf, TIME_INFINITE);

			// Start the decoding thread when enough data have been received.
			initial_buffering--;
			if (initial_buffering == 0)
				chBSemSignal(&decode_bsem);
			chMtxUnlock(&audio_mutex);
		}
		chMtxLock(&audio_mutex);
		chMBFetch(&free_input_box, (msg_t*)&inbuf, TIME_INFINITE);
		inbuf = NULL;
		chMBPost(&input_box, (msg_t)inbuf, TIME_INFINITE);
		chMtxUnlock(&audio_mutex);
	}
}

THD_WORKING_AREA(wa_flash, 128);
// To use for alarm clock
THD_FUNCTION(flash_audio_in, arg) {
	UNUSED(arg);

	/**
	 * Pointer to the position of the next data to read (in music_buffer).
	 */
	static const int8_t* cur_pos;

	/**
	 * Pointer to buffers used in the mailbox.
	 */
	static void* inbuf;

	/**
	 * Id of the 'inner' music currently played.
	 */
	static int cur_id;

	while(true) {
		chBSemWait(&audio_bsem);
		chMtxLock(&audio_mutex);
		reset_mailboxes();
		// Start to play music.
		cur_id = music_id;
		chBSemSignal(&decode_bsem);
		do {
			cur_pos = _binary_start[cur_id];
			while (cur_pos + INPUT_BUFFER_SIZE < _binary_end[cur_id]) {
				chMBFetch(&free_input_box, (msg_t*)&inbuf, TIME_INFINITE);
				memcpy(inbuf, cur_pos, INPUT_BUFFER_SIZE);
				cur_pos += INPUT_BUFFER_SIZE;
				chMBPost(&input_box, (msg_t)inbuf, TIME_INFINITE);
				if (!repeat)
					break;
			}
		} while(repeat);
		chMBFetch(&free_input_box, (msg_t*)&inbuf, TIME_INFINITE);
		inbuf = NULL;
		chMBPost(&input_box, (msg_t)inbuf, TIME_INFINITE);
		chMtxUnlock(&audio_mutex);
	}
}
