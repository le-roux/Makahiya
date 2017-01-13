#include "ch.h"
#include "hal.h"
#include <math.h>
#include "utils.h"
#include <string.h>
#include "chprintf.h"
#include "usbcfg.h"

#include "sound.h"
#include "wifi.h"

int16_t i2s_tx_buf[I2S_BUF_SIZE * 2];
thread_reference_t audio_thread_ref = NULL;

static int16_t buf[AUDIO_BUFFERS_NB][I2S_BUF_SIZE];
static msg_t audio_buffers[AUDIO_BUFFERS_NB];
static msg_t free_audio_buffers[AUDIO_BUFFERS_NB];
MAILBOX_DECL(audio_box, audio_buffers, AUDIO_BUFFERS_NB);
MAILBOX_DECL(free_box, free_audio_buffers, AUDIO_BUFFERS_NB);


static int16_t in_buf[INPUT_BUFFERS_NB][INPUT_BUFFER_SIZE];
static msg_t input_buffers[INPUT_BUFFERS_NB];
static msg_t free_input_buffers[INPUT_BUFFERS_NB];
MAILBOX_DECL(input_box, input_buffers, INPUT_BUFFERS_NB);
MAILBOX_DECL(free_input_box, free_input_buffers, INPUT_BUFFERS_NB);

static SEMAPHORE_DECL(audio_sem, 2);
BSEMAPHORE_DECL(audio_bsem, true);
BSEMAPHORE_DECL(decode_bsem, true);

static int8_t working_buffer[WORKING_BUFFER_SIZE];

volatile int count = 0;
volatile bool started = false;

int8_t volumeMult = 1;
int8_t volumeDiv = 1;

I2SConfig i2s3_cfg = {
    i2s_tx_buf,
    NULL,
    I2S_BUF_SIZE * 2,
    i2s_cb,
    0,
    SPI_I2SPR_MCKOE | I2SDIV
};

void sound_set_pins(void) {
    palSetPadMode(GPIOA, 15, PAL_MODE_ALTERNATE(6));
    palSetPadMode(GPIOB, 3, PAL_MODE_ALTERNATE(6));
    palSetPadMode(GPIOB, 5, PAL_MODE_ALTERNATE(6));
    palSetPadMode(GPIOC, 7, PAL_MODE_ALTERNATE(6));
}

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
    int bytes_left = 0, count = 0;
    HMP3Decoder decoder;
    unsigned char* read_ptr;
    volatile int offset, err;
    void* pbuf, *inbuf;
    UNUSED(arg);

    // Init the free buffers mailbox
    for (int i = 0; i < AUDIO_BUFFERS_NB; i++)
        (void)chMBPost(&free_box, (msg_t)&buf[i], TIME_INFINITE);

    decoder = MP3InitDecoder();

    chBSemWait(&decode_bsem);

    while (TRUE) {
        // Get a free buffer (for output)
        if (chMBFetch(&free_box, (msg_t*)&pbuf, TIME_INFINITE) != MSG_OK) {
            chThdSleepMilliseconds(20);
            continue;
        }

        // Acquire new data if possible
        while (WORKING_BUFFER_SIZE - bytes_left >= 2 * INPUT_BUFFER_SIZE) { // Space available for new data
            // Get an input buffer
            chMBFetch(&input_box, (msg_t*)&inbuf, TIME_INFINITE);

            if (inbuf == NULL) { // End of music
                chMBPost(&free_box, (msg_t)pbuf, TIME_INFINITE);
                chMBPost(&free_input_box, (msg_t)inbuf, TIME_INFINITE);
                i2sStopExchange(&I2SD3);
                i2sStop(&I2SD3);
                started = false;
                return;
            }

            // Copy the new data at the end of the working buffer
            memcpy(&working_buffer[bytes_left], inbuf, INPUT_BUFFER_SIZE * 2);
            bytes_left += INPUT_BUFFER_SIZE * 2;

            // Release the input buffer
            chMBPost(&free_input_box, (msg_t)inbuf, TIME_INFINITE);
        }

        read_ptr = (unsigned char*)working_buffer;
        offset = MP3FindSyncWord(read_ptr, bytes_left);
        bytes_left -=  offset;
        memmove(working_buffer, working_buffer + offset, bytes_left);

        // Decode the mp3 block
        err = MP3Decode(decoder, &read_ptr, &bytes_left, (int16_t*)pbuf, 0);

        memmove(working_buffer, read_ptr, bytes_left);

        if (err == -6) { // TODO: take care of other errors
            chMBPost(&free_box, (msg_t)pbuf, TIME_INFINITE);
            DEBUG("stop err= %i", err);
            if (I2SD3.state != I2S_STOP) {
                chThdSleepMilliseconds(100);
                i2sStopExchange(&I2SD3);
                i2sStop(&I2SD3);
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
            i2sStart(&I2SD3, &i2s3_cfg);
            i2sStartExchange(&I2SD3);
            started = true;
        } else {
            chSysUnlock();
        }
    }
}

THD_WORKING_AREA(wa_audio_in, 2048);

THD_FUNCTION(wifi_audio_in, arg) {
    UNUSED(arg);
    void* inbuf;
    int bytes_nb, bytes_consumed, copy, initial_buffering = 0;
    wifi_response_header out;

    // Init the free input buffers mailbox
    for (int i = 0; i < INPUT_BUFFERS_NB; i++)
        chMBPost(&free_input_box, (msg_t)&in_buf[i], TIME_INFINITE);

    while (TRUE) {
        bytes_consumed = WIFI_BUFFER_SIZE;
        out.length = WIFI_BUFFER_SIZE;

        chBSemWait(&audio_bsem);
        DEBUG("start reading audio");
        while (TRUE) {
            // Get a free buffer
            chMBFetch(&free_input_box, (msg_t*)&inbuf, TIME_INFINITE);

            // Read file from wifi
            bytes_nb = 0;
            bytes_consumed = WIFI_BUFFER_SIZE;
            out.length = WIFI_BUFFER_SIZE;
            out.error = 0;
            out.error_code = 0;

            while(bytes_nb < INPUT_BUFFER_SIZE * 2) {
                DEBUG("enter while");
                while (bytes_consumed >= out.length) { // Need to perform a new read.
                    bytes_consumed = 0;
                    read_buffer(audio_conn);
                    out = get_response(true);
                    if (out.error && out.error_code == NO_DATA)
                        break;
                }
                if (out.error && out.error_code == NO_DATA)
                    break;
                /**
                 * copy = min(out.length - bytes_consumed,
                 *            2 * INPUT_BUFFER_SIZE - bytes_nb)
                 */
                copy = out.length - bytes_consumed;
                if (2 * INPUT_BUFFER_SIZE - bytes_nb < copy)
                    copy = 2 * INPUT_BUFFER_SIZE - bytes_nb;
                // Don't copy more than remaining space.
                memcpy(&((int8_t*)inbuf)[bytes_nb], &response_body[bytes_consumed], copy);
                bytes_nb += copy;
                bytes_consumed += copy;
                chDbgCheck(bytes_nb <= INPUT_BUFFER_SIZE * 2);
                chDbgCheck(bytes_consumed <= out.length);
            }
            if (out.error && out.error_code == NO_DATA) {
                chMBPost(&free_input_box, (msg_t)inbuf, TIME_INFINITE);
                break;
            }
            DEBUG("post");

            chMBPost(&input_box, (msg_t)inbuf, TIME_INFINITE);
            initial_buffering++;
            if (initial_buffering == 20)
                chBSemSignal(&decode_bsem);
        }
        if (out.error && out.error_code == NO_DATA)
            break;
        chMBFetch(&free_input_box, (msg_t*)&inbuf, TIME_INFINITE);
        inbuf = NULL;
        chMBPost(&input_box, (msg_t)inbuf, TIME_INFINITE);
    }

}

THD_FUNCTION(flash_audio_in, arg) {
    UNUSED(arg);
    int16_t* cur_pos = &_binary_music_mp3_start;
    int8_t* write_pos = (int8_t*)&_binary_music_mp3_start;
    void* inbuf;

    wifi_response_header out = {0, 0, 0};
    chBSemWait(&audio_bsem);
    /*while (write_pos + WIFI_BUFFER_SIZE < (int8_t*)&_binary_music_mp3_end) {
        // Download file
        out.length = 0;
        while (out.length == 0) { // Need to perform a new read.
            read_buffer(audio_conn);
            out = get_response(true);
            if (out.error && out.error_code == NO_DATA)
                break;
        }
        DEBUG("memcpy %i %x", out.length, write_pos);
        memcpy(write_pos, response_body, out.length);
        write_pos += out.length;
    }*/

    cur_pos = &_binary_music_mp3_start;
    chBSemSignal(&decode_bsem);
    // Init the free input buffers mailbox
    for (int i = 0; i < INPUT_BUFFERS_NB; i++)
        chMBPost(&free_input_box, (msg_t)&in_buf[i], TIME_INFINITE);

    while (cur_pos + INPUT_BUFFER_SIZE < &_binary_music_mp3_end) {
        if (chMBFetch(&free_input_box, (msg_t*)&inbuf, TIME_INFINITE) != MSG_OK)
            break;
        memcpy(inbuf, cur_pos, 2 * INPUT_BUFFER_SIZE);
        cur_pos += INPUT_BUFFER_SIZE;

        chMBPost(&input_box, (msg_t)inbuf, TIME_INFINITE);
    }
    chMBFetch(&free_input_box, (msg_t*)&inbuf, TIME_INFINITE);
    inbuf = NULL;
    chMBPost(&input_box, (msg_t)inbuf, TIME_INFINITE);
}
