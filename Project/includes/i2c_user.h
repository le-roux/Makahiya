#ifndef I2C_USER_H
#define I2C_USER_H

#define I2C_RX_BUFFER_SIZE 16
#define I2C_TX_BUFFER_SIZE 16

#define TIMEOUT_I2C MS2ST(4)

/**
 * @brief Configuration for the I2C driver 1 (the one used for the capacitive
 *          sensor).
 */
extern I2CConfig i2c1_cfg;

extern uint8_t i2c_rx_buffer[I2C_RX_BUFFER_SIZE];
extern uint8_t i2c_tx_buffer[I2C_TX_BUFFER_SIZE];

/**
 * @brief Set the pins used by the I2C bus in the proper configuration.
 */
void i2c_set_pins(void);

#endif
