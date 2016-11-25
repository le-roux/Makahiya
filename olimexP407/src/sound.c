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
static msg_t free_buffers[AUDIO_BUFFERS_NB];
MAILBOX_DECL(audio_box, audio_buffers, AUDIO_BUFFERS_NB);
MAILBOX_DECL(free_box, free_buffers, AUDIO_BUFFERS_NB);

int8_t volumeMult = 1;
int8_t volumeDiv = 2;

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

THD_WORKING_AREA(wa_audio, 512);

THD_FUNCTION(audio_playback, arg) {
    static const char* read_ptr = (char*)&_binary_pic_mp3_start;
    static int bytes_left = (int)&_binary_pic_mp3_size;
    HMP3Decoder decoder;
    int offset, err;
    void* pbuf;
    UNUSED(arg);

    // Init the free buffers mailbox
    for (int i = 0; i < AUDIO_BUFFERS_NB; i++)
        (void)chMBPost(&free_box, (msg_t)&buf[i], TIME_INFINITE);

    decoder = MP3InitDecoder();

    i2sStart(&I2SD3, &i2s3_cfg);
    i2sStartExchange(&I2SD3);

    while (TRUE) {
        // Get a free buffer
        if (chMBFetch(&free_box, (msg_t*)&pbuf, TIME_INFINITE) != MSG_OK) {
            chThdSleepMilliseconds(20);
            continue;
        }
        // Decode the mp3 block
        offset = MP3FindSyncWord((unsigned char*)read_ptr, bytes_left);
        read_ptr += offset;
        bytes_left -= offset;

        err = MP3Decode(decoder, (unsigned char**)&read_ptr, &bytes_left, (int16_t*)pbuf, 0);
        if (err != 0) {
            i2sStopExchange(&I2SD3);
            i2sStop(&I2SD3);
            break;// TODO improve error management
        }

        // Post the filled buffer
        chMBPost(&audio_box, (msg_t)pbuf, TIME_INFINITE);
    }
}
