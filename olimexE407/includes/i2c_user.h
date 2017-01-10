#ifndef I2C_USER_H
#define I2C_USER_H

#define RX_BUFFER_SIZE 16
#define TX_BUFFER_SIZE 16

#define TIMEOUT MS2ST(4)

#define DEVICE_ID_AADDR 0x7F
#define DEVICE_ID_L 0x55
#define DEVICE_ID_H 0x30

extern I2CConfig i2c2_cfg;
extern uint8_t rx_buffer[RX_BUFFER_SIZE];
extern uint8_t tx_buffer[TX_BUFFER_SIZE];


void i2c_set_pins(void);

#endif
