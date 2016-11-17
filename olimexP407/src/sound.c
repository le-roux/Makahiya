#include "ch.h"
#include "hal.h"
#include <math.h>
#include "utils.h"
#include <string.h>

#include "sound.h"

const I2SConfig i2s3_cfg = {
    i2s_tx_buf,
    NULL,
    I2S_BUF_SIZE,
    i2s_cb,
    0,
    SPI_I2SPR_MCKOE | I2SDIV | SPI_I2SPR_ODD
};

void sound_set_pins(void) {
    palSetPadMode(GPIOA, 15, PAL_MODE_ALTERNATE(6));
    palSetPadMode(GPIOB, 3, PAL_MODE_ALTERNATE(6));
    palSetPadMode(GPIOB, 5, PAL_MODE_ALTERNATE(6));
    palSetPadMode(GPIOC, 7, PAL_MODE_ALTERNATE(6));
}

THD_WORKING_AREA(i2s_space, 20480);
static MAILBOX_DECL(i2s_queue, i2s_space, sizeof(i2s_space) / sizeof(msg_t));

CH_IRQ_HANDLER(Vector10C) {
  CH_IRQ_PROLOGUE();
  static int right = 0;
  static msg_t sample;
  if (right) {
    SPI3->DR = sample >> 16;
    right = 0;
  } else {
    chSysLockFromISR();
    if (chMBFetchI(&i2s_queue, &sample) == MSG_OK) {
      SPI3->DR = sample;
      right = 1;
    } else
      SPI3->CR2 &= ~SPI_CR2_TXEIE;
    chSysUnlockFromISR();
  }
  CH_IRQ_EPILOGUE();
}

void play_sample(uint16_t left, uint16_t right) {
  chMBPost(&i2s_queue, (0 << 16) | left, TIME_INFINITE);
  SPI3->CR2 |= SPI_CR2_TXEIE;
}

void sound_init(void) {
  RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
  RCC->PLLI2SCFGR |= (2 << 28) | (213 << 6);
  RCC->CR |= RCC_CR_PLLI2SON;
  while (!(RCC->CR & RCC_CR_PLLI2SRDY)) ;
  SPI3->I2SCFGR = SPI_I2SCFGR_I2SMOD | SPI_I2SCFGR_I2SCFG_1;
  SPI3->I2SPR = SPI_I2SPR_MCKOE | SPI_I2SPR_ODD | 6;
  SPI3->I2SCFGR |= SPI_I2SCFGR_I2SE;
  NVIC_SetPriority(51, 251);
  NVIC_EnableIRQ(51);
}

void sound_440(void) {
  extern int16_t _binary_pic_pcm_start;
  extern int16_t _binary_pic_pcm_end;
  int i = 0;
  for (;;) {
    for (int16_t *p = &_binary_pic_pcm_start;
	 p < &_binary_pic_pcm_end;
	 p++) {
      i++;
      int16_t sample = 32767 * sin(i * 440 * 2 * M_PI / 32000);
      play_sample(*p, sample);
    }
  }
}

void i2s_cb(I2SDriver* driver, size_t offset, size_t n) {
    UNUSED(driver);
    UNUSED(n);
    // Play in loop
    if (sound_index + I2S_BUF_SIZE / 2 > (uint32_t)&_binary_pic_pcm_size / 2)
        sound_index = 0;

    for (int i = 0; i < I2S_BUF_SIZE / 2; i++) {
        i2s_tx_buf[offset + i] = 0x0000FFFF & (&_binary_pic_pcm_start)[sound_index + i];
    }
    sound_index += I2S_BUF_SIZE / 2;
}