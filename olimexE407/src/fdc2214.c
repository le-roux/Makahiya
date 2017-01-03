#include "fdc2214.h"
#include "i2c_user.h"

uint16_t config = CONFIG_RESERVED;

msg_t write_register(uint8_t addr, uint8_t reg_addr, uint16_t value) {
    tx_buffer[0] = reg_addr;
    tx_buffer[1] = (value >> 8) & 0xFF;
    tx_buffer[2] = value & 0xFF;
    return i2cMasterTransmitTimeout(&I2CD2, addr, tx_buffer, 3, NULL, 0, TIMEOUT);
}

msg_t read_register(uint8_t addr, uint8_t reg_addr) {
    tx_buffer[0] = reg_addr;
    return i2cMasterTransmitTimeout(&I2CD2, addr, tx_buffer, 1, rx_buffer, 2, TIMEOUT);
}
