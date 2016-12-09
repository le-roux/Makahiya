#ifndef USART_H
#define USART_H

#include "ch.h"
#include "hal.h"

/***********************/
/*       Variables     */
/***********************/
#define UART_BUF_SIZE 8
extern UARTConfig uart3_cfg;
extern uint8_t uart_tx_buffer[UART_BUF_SIZE];
extern uint8_t uart_rx_buffer[UART_BUF_SIZE];

/***********************/
/*       Functions     */
/***********************/
void txend1_cb(UARTDriver* driver);
void txend2_cb(UARTDriver* driver);
void rxend_cb(UARTDriver* driver);
void rxchar_cb(UARTDriver* driver, uint16_t c);
void rxerr_cb(UARTDriver* driver, uartflags_t flags);

void uart_set_pins(void);

#endif // USART_H
