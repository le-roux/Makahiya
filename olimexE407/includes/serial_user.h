#ifndef SERIAL_USER_H
#define SERIAL_USER_H

#include "ch.h"
#include "hal.h"

#define SERIAL_TX_BUFFER_SIZE 256
#define SERIAL_RX_BUFFER_SIZE 256

extern uint8_t serial_tx_buffer[SERIAL_TX_BUFFER_SIZE];
extern uint8_t serial_rx_buffer[SERIAL_RX_BUFFER_SIZE];
extern SerialConfig serial_cfg;
extern SerialDriver* bluetooth_SD;

void serial_set_pin(void);

#endif // SERIAL_USER_H
