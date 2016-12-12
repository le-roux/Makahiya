#include "ch.h"
#include "hal.h"
#include <math.h>
#include "utils.h"
#include <string.h>
#include "chprintf.h"
#include "usbcfg.h"

#include "sound.h"

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
    UNUSED(n);
    void* pbuf;
    msg_t ret;
    chSysLockFromISR();

    // Get a filled buffer
    ret = chMBFetchI(&audio_box, (msg_t*)&pbuf);
    if (ret != MSG_OK)
        palClearPad(GPIOF, 6);

    // Copy the data in the sending buffer and adjust the volume
    for (int i = 0; i < I2S_BUF_SIZE; i++)
        i2s_tx_buf[offset + i] = ((int16_t*)pbuf)[i] * volumeMult / volumeDiv;

    // Release the buffer
    chMBPostI(&free_box, (msg_t)pbuf);
    chSysUnlockFromISR();
}

THD_WORKING_AREA(wa_audio, 1024);

THD_FUNCTION(audio_playback, arg) {
    int bytes_left = 0;
    HMP3Decoder decoder;
    unsigned char* read_ptr;
    volatile int offset, err;
    void* pbuf, *inbuf;
    UNUSED(arg);

    // Init the free buffers mailbox
    for (int i = 0; i < AUDIO_BUFFERS_NB; i++)
        (void)chMBPost(&free_box, (msg_t)&buf[i], TIME_INFINITE);

    decoder = MP3InitDecoder();

    while (TRUE) {
        // Get a free buffer (for output)
        if (chMBFetch(&free_box, (msg_t*)&pbuf, TIME_INFINITE) != MSG_OK) {
            chThdSleepMilliseconds(20);
            continue;
        }

        // Acquire new data if possible
        while (WORKING_BUFFER_SIZE - bytes_left > 2 * INPUT_BUFFER_SIZE) { // Space available for new data
            // Get an input buffer
            if (chMBFetch(&input_box, (msg_t*)&inbuf, TIME_INFINITE) != MSG_OK) {
                chThdSleepMilliseconds(20);
                chMBPost(&free_box, (msg_t)pbuf, TIME_INFINITE);
                continue;
            }

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

        if (!started) {
            i2sStart(&I2SD3, &i2s3_cfg);
            i2sStartExchange(&I2SD3);
            started = true;
        }
        read_ptr = (unsigned char*)working_buffer;
        offset = MP3FindSyncWord(read_ptr, bytes_left);
        bytes_left -=  offset;
        for (int i = 0; i < WORKING_BUFFER_SIZE - offset; i++)
            working_buffer[i] = working_buffer[i + offset]; // Erase the non-interesting part

        // Decode the mp3 block
        err = MP3Decode(decoder, &read_ptr, &bytes_left, (int16_t*)pbuf, 0);

        for (int i = 0; i < bytes_left; i++)
            working_buffer[i] = read_ptr[i];

        if (err != 0) {
            chMBPost(&free_box, (msg_t)pbuf, TIME_INFINITE);

            i2sStopExchange(&I2SD3);
            i2sStop(&I2SD3);
            started = false;
            continue;// TODO improve error management
        }

        // Post the filled buffer
        chMBPost(&audio_box, (msg_t)pbuf, TIME_INFINITE);
    }
}

THD_WORKING_AREA(wa_audio_in, 2048);

THD_FUNCTION(audio_in, arg) {
    UNUSED(arg);
    int bytes_nb;
    void* inbuf;

    // Init the free input buffers mailbox
    for (int i = 0; i < INPUT_BUFFERS_NB; i++)
        chMBPost(&free_input_box, (msg_t)&in_buf[i], TIME_INFINITE);

    chThdSleepMilliseconds(100);

    while (chSequentialStreamGet((BaseSequentialStream*)&SDU1) == -2) {
        chThdSleepMilliseconds(10);
    }

    while (TRUE) {
        // Get a free buffer
        if (chMBFetch(&free_input_box, (msg_t*)&inbuf, TIME_INFINITE) != MSG_OK) {
            chThdSleepMilliseconds(100);
            continue;
        }

        // Read file from serial link
        bytes_nb = 0;
        while(bytes_nb < INPUT_BUFFER_SIZE * 2) {
            ((int8_t*)inbuf)[bytes_nb] = chSequentialStreamGet((BaseSequentialStream*)&SDU1);
            bytes_nb++;
        }

        chMBPost(&input_box, (msg_t)inbuf, TIME_INFINITE);
    }
    chMBFetch(&free_input_box, (msg_t*)&inbuf, TIME_INFINITE);
    inbuf = NULL;
    chMBPost(&input_box, (msg_t)inbuf, TIME_INFINITE);

}
