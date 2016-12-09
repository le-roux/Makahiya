#include "usart.h"
#include "utils.h"
#include "logging.h"
#include <string.h>

/***********************/
/*       Variables     */
/***********************/
UARTConfig uart3_cfg = {
    txend1_cb,
    txend2_cb,
    rxend_cb,
    rxchar_cb,
    rxerr_cb,
    38400,
    0,
    0,
    0
};

uint8_t uart_tx_buffer[UART_BUF_SIZE];
uint8_t uart_rx_buffer[UART_BUF_SIZE];


/***********************/
/*       Functions     */
/***********************/
void uart_set_pins(void) {
    for (int i = 0; i < UART_BUF_SIZE; i++)
        uart_tx_buffer[i] = 'a';
    palSetPadMode(GPIOC, 10, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOC, 11, PAL_MODE_ALTERNATE(7));
}


void txend1_cb(UARTDriver* driver) {
    UNUSED(driver);
}

void txend2_cb(UARTDriver* driver) {
    UNUSED(driver);
}

void rxend_cb(UARTDriver* driver) {
    UNUSED(driver);
    memcpy(logging_buffer, uart_rx_buffer, UART_BUF_SIZE);
    logging_buffer[LOGGING_BUFFER_SIZE - 1] = '\0';
    chSysLockFromISR();
    chBSemSignalI(&logging_bsem);
    chSysUnlockFromISR();
}

void rxchar_cb(UARTDriver* driver, uint16_t c) {
    UNUSED(driver);
    UNUSED(c);
}

void rxerr_cb(UARTDriver* driver, uartflags_t flags) {
    UNUSED(driver);
    UNUSED(flags);
}
