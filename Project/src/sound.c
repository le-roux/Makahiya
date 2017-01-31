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
	&_binary_alarm3_mp3_start};

const int8_t* const _binary_size[ALARM_SOUND_NB] = {&_binary_alarm1_mp3_size,
	&_binary_alarm2_mp3_size,
	&_binary_alarm3_mp3_size};

const int8_t* const _binary_end[ALARM_SOUND_NB] = {&_binary_alarm1_mp3_end,
	&_binary_alarm2_mp3_end,
	&_binary_alarm3_mp3_end};


volatile int music_id;
volatile int repeat;
volatile wifi_connection audio_conn;

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
#define INPUT_BUFFERS_NB 10

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

volatile int count = 0;
volatile bool started = false;

void audioInit(void){
	palSetPadMode(GPIOB, 12,  PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOB, 13,  PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOB, 15,  PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOC, 6,  PAL_MODE_ALTERNATE(5));
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
	/**
	 * Number of bytes not yet consumed by the decoder in the working buffer.
	 */
	int bytes_left = 0;

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
	unsigned char* read_ptr;

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

	int buf_nb;
	unsigned char *tmp_cur;
	UNUSED(arg);

	// Init the free buffers mailbox
	for (int i = 0; i < DECODED_DATA_BUFFERS_NB; i++)
		(void)chMBPost(&free_decoded_data_box, (msg_t)&buf[i], TIME_INFINITE);

	decoder = MP3InitDecoder();

	while(true) {
		ended = false;
		// Wait to be triggered by one of the reading threads.
		chBSemWait(&decode_bsem);

		while (true) {
			// Get a free buffer (for output)
			chMBFetch(&free_decoded_data_box, (msg_t*)&pbuf, TIME_INFINITE);

			// Acquire new data if possible
			while (WORKING_BUFFER_SIZE - bytes_left >= INPUT_BUFFER_SIZE) { // While there is space available for new data
				chSysLock();
				buf_nb = chMBGetUsedCountI(&input_box);
				chSysUnlock();
				DEBUG("buf nb %i", buf_nb);

				// Don't wait if there is no buffer available and still enough data.
				if (buf_nb == 0 && bytes_left > WORKING_BUFFER_MIN_LENGTH)
					break;

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
				memcpy(&working_buffer[bytes_left], inbuf, INPUT_BUFFER_SIZE);
				bytes_left += INPUT_BUFFER_SIZE;

				// Release the input buffer
				chMBPost(&free_input_box, (msg_t)inbuf, TIME_INFINITE);
			}
			if (ended)
				break;

			read_ptr = (unsigned char*)working_buffer;
			offset = MP3FindSyncWord(read_ptr, bytes_left);
			DEBUG("offset %i (length %i)", offset, bytes_left);

			if (offset < 0) { // No sync word in the working buffer: no more interesting data.
				offset = 0;
				bytes_left = 0;
				continue;
			} else { // Normal behaviour.
				bytes_left -=  offset;
			}
			memmove(working_buffer, working_buffer + offset, bytes_left);

			// Decode the mp3 block
			tmp_cur = read_ptr;
			err = MP3Decode(decoder, &read_ptr, &bytes_left, (int16_t*)pbuf, 0);
			DEBUG("decoded %i", read_ptr - tmp_cur);
			memmove(working_buffer, read_ptr, bytes_left);

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
				bytes_left = 0;
				read_ptr = (unsigned char*)working_buffer;
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
	 * Number of bytes already read in the current frame.
	 */
	static int bytes_consumed;

	/**
	 * Number of bytes to copy from the input frame to the internal buffer.
	 */
	static int copy_size;

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

	// Init the free input buffers mailbox
	for (int i = 0; i < INPUT_BUFFERS_NB; i++)
		chMBPost(&free_input_box, (msg_t)&in_buf[i], TIME_INFINITE);

	while (true) {
		// Wait to be triggered by the user.
		chBSemWait(&download_bsem);

		count_nodata = 0;
		initial_buffering = INPUT_BUFFERS_NB / 2;
		bytes_consumed = INPUT_BUFFER_SIZE;
		out.length = INPUT_BUFFER_SIZE;
		out.error = 0;
		out.error_code = NO_ERROR;
		while (count_nodata < 6) {
			chMBFetch(&free_input_box, (msg_t*)&inbuf, TIME_INFINITE);

			bytes_nb = 0;

			// Read file from wifi
			while(bytes_nb < INPUT_BUFFER_SIZE) { // while buffer is not full.
				while (bytes_consumed >= out.length) { // Need to perform a new read.
					bytes_consumed = 0;
					out = read_buffer(audio_conn, true);
					if (out.error && out.error_code == NO_DATA) {
						count_nodata++;
						chThdSleepMilliseconds(1000);
					} else {
						count_nodata = 0;
					}
				}

				/**
				 * copy_size = min(out.length - bytes_consumed,
				 *            INPUT_BUFFER_SIZE - bytes_nb)
				 */
				copy_size = out.length - bytes_consumed;
				if (INPUT_BUFFER_SIZE - bytes_nb < copy_size)
					copy_size = INPUT_BUFFER_SIZE - bytes_nb;

				// Don't copy more than remaining space.
				memcpy((uint8_t*)inbuf + bytes_nb, &response_body[bytes_consumed], copy_size);
				bytes_nb += copy_size;
				bytes_consumed += copy_size;
			}
			chMBPost(&input_box, (msg_t)inbuf, TIME_INFINITE);
			initial_buffering--;
			if (initial_buffering == 0)
				chBSemSignal(&decode_bsem);
		}
		chMBFetch(&free_input_box, (msg_t*)&inbuf, TIME_INFINITE);
		inbuf = NULL;
		chMBPost(&input_box, (msg_t)inbuf, TIME_INFINITE);
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

	// Init the free input buffers mailbox
	for (int i = 0; i < INPUT_BUFFERS_NB; i++)
		chMBPost(&free_input_box, (msg_t)&in_buf[i], TIME_INFINITE);

	while(TRUE) {
		chBSemWait(&audio_bsem);
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
	}
}
