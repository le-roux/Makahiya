#include "fdc2214.h"
#include "i2c_user.h"

uint16_t config = CONFIG_RESERVED;
uint16_t status;

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

i2cflags_t init_sensor(void) {
    msg_t status;

    // Set the sensor in SLEEP_MODE
    status = write_register(FDC1_ADDR, CONFIG, CONFIG_RESERVED | SLEEP_MODE);
    if (status != MSG_OK)
        return i2cGetErrors(&I2CD2);

    // Set the clock divider registers
    status = write_register(FDC1_ADDR, CLOCK_DIVIDERS_0, 0x1001);
    status = write_register(FDC1_ADDR, CLOCK_DIVIDERS_1, 0x1001);
    status = write_register(FDC1_ADDR, CLOCK_DIVIDERS_2, 0x1001);
    status = write_register(FDC1_ADDR, CLOCK_DIVIDERS_3, 0x1001);

    // Set the drive current registers
    status = write_register(FDC1_ADDR, DRIVE_CURRENT_CH0, 0x8C40);
    status = write_register(FDC1_ADDR, DRIVE_CURRENT_CH1, 0x8C40);
    status = write_register(FDC1_ADDR, DRIVE_CURRENT_CH2, 0x8800);
    status = write_register(FDC1_ADDR, DRIVE_CURRENT_CH3, 0x8800);

    // Set the settlecount registers
    status = write_register(FDC1_ADDR, SETTLECOUNT_CH0, 0x0400);
    status = write_register(FDC1_ADDR, SETTLECOUNT_CH1, 0x0400);
    status = write_register(FDC1_ADDR, SETTLECOUNT_CH2, 0x0400);
    status = write_register(FDC1_ADDR, SETTLECOUNT_CH3, 0x0400);

    // Set the rcount registers
    status = write_register(FDC1_ADDR, RCOUNT_CH0, 0xFFFF);
    status = write_register(FDC1_ADDR, RCOUNT_CH1, 0xFFFF);
    status = write_register(FDC1_ADDR, RCOUNT_CH2, 0xFFFF);
    status = write_register(FDC1_ADDR, RCOUNT_CH3, 0xFFFF);

    // Set the offset registers
    status = write_register(FDC1_ADDR, OFFSET_CH0, 0x0000);
    status = write_register(FDC1_ADDR, OFFSET_CH1, 0x0000);
    status = write_register(FDC1_ADDR, OFFSET_CH2, 0x0000);
    status = write_register(FDC1_ADDR, OFFSET_CH3, 0x0000);

    // Set the status config register
    status = write_register(FDC1_ADDR, STATUS_CONFIG, 0x0001);

    // Set the mux_config register
    status = write_register(FDC1_ADDR, MUX_CONFIG, 0xC209);

    // Set the config register (and leave sleep mode)
    status = write_register(FDC1_ADDR, CONFIG, 0x1C01);

    return I2C_NO_ERROR;
}
