#include "fdc2214.h"
#include "i2c_user.h"
#include "ext_user.h"
#include "utils.h"
#include "capacitive_sensor.h"

#include "chprintf.h"
#include "RTT_streams.h"

volatile uint8_t calling;
static int FDC_ADDR[2] = {FDC1_ADDR, FDC2_ADDR};
static int DATA_MSB[4] = {DATA_MSB_CH0, DATA_MSB_CH1, DATA_MSB_CH2, DATA_MSB_CH3};
static int DATA_LSB[4] = {DATA_LSB_CH0, DATA_LSB_CH1, DATA_LSB_CH2, DATA_LSB_CH3};
static int CHANNELS_NB[2] = {4, 4};
static msg_t sensors[2];
static MAILBOX_DECL(sensors_box, sensors, 2);

BSEMAPHORE_DECL(fdc_bsem, true);

static msg_t write_register(uint8_t addr, uint8_t reg_addr, uint16_t value) {
	i2c_tx_buffer[0] = reg_addr;
	i2c_tx_buffer[1] = (value >> 8) & 0xFF;
	i2c_tx_buffer[2] = value & 0xFF;
	return i2cMasterTransmitTimeout(&I2CD1, addr, i2c_tx_buffer, 3, NULL, 0, TIMEOUT_I2C);
}

static msg_t read_register(uint8_t addr, uint8_t reg_addr) {
	i2c_tx_buffer[0] = reg_addr;
	return i2cMasterTransmitTimeout(&I2CD1, addr, i2c_tx_buffer, 1, i2c_rx_buffer, 2, TIMEOUT_I2C);
}

static i2cflags_t init_sensors(void) {
	msg_t status;

	// Set the sensor in SLEEP_MODE
	status = write_register(FDC1_ADDR, CONFIG, CONFIG_RESERVED | SLEEP_MODE);
	if (status != MSG_OK)
		return i2cGetErrors(&I2CD1);

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

	// Set the status/error config register
	status = write_register(FDC1_ADDR, STATUS_CONFIG, 0x0001);

	// Set the mux_config register
	status = write_register(FDC1_ADDR, MUX_CONFIG, 0xC209);

	// Set the config register (and leave sleep mode)
	status = write_register(FDC1_ADDR, CONFIG, 0x1C01);

	// Set the sensor in SLEEP_MODE
	status = write_register(FDC2_ADDR, CONFIG, CONFIG_RESERVED | SLEEP_MODE);
	if (status != MSG_OK)
		return i2cGetErrors(&I2CD1);

	// Set the clock divider registers
	status = write_register(FDC2_ADDR, CLOCK_DIVIDERS_0, 0x1001);
	status = write_register(FDC2_ADDR, CLOCK_DIVIDERS_1, 0x1001);
	status = write_register(FDC2_ADDR, CLOCK_DIVIDERS_2, 0x1001);
	status = write_register(FDC2_ADDR, CLOCK_DIVIDERS_3, 0x1001);

	// Set the drive current registers
	status = write_register(FDC2_ADDR, DRIVE_CURRENT_CH0, 0x8C40);
	status = write_register(FDC2_ADDR, DRIVE_CURRENT_CH1, 0x8C40);
	status = write_register(FDC2_ADDR, DRIVE_CURRENT_CH2, 0x8800);
	status = write_register(FDC2_ADDR, DRIVE_CURRENT_CH3, 0x8800);

	// Set the settlecount registers
	status = write_register(FDC2_ADDR, SETTLECOUNT_CH0, 0x0400);
	status = write_register(FDC2_ADDR, SETTLECOUNT_CH1, 0x0400);
	status = write_register(FDC2_ADDR, SETTLECOUNT_CH2, 0x0400);
	status = write_register(FDC2_ADDR, SETTLECOUNT_CH3, 0x0400);

	// Set the rcount registers
	status = write_register(FDC2_ADDR, RCOUNT_CH0, 0xFFFF);
	status = write_register(FDC2_ADDR, RCOUNT_CH1, 0xFFFF);
	status = write_register(FDC2_ADDR, RCOUNT_CH2, 0xFFFF);
	status = write_register(FDC2_ADDR, RCOUNT_CH3, 0xFFFF);

	// Set the offset registers
	status = write_register(FDC2_ADDR, OFFSET_CH0, 0x0000);
	status = write_register(FDC2_ADDR, OFFSET_CH1, 0x0000);
	status = write_register(FDC2_ADDR, OFFSET_CH2, 0x0000);
	status = write_register(FDC2_ADDR, OFFSET_CH3, 0x0000);

	// Set the status/error config register
	status = write_register(FDC2_ADDR, STATUS_CONFIG, 0x0001);

	// Set the mux_config register
	status = write_register(FDC2_ADDR, MUX_CONFIG, 0xC209);

	// Set the config register (and leave sleep mode)
	status = write_register(FDC2_ADDR, CONFIG, 0x1C01);

	return I2C_NO_ERROR;
}

/**
 * @brief Acquire data from one channel of one sensor and check if an event has
 *			occurred, performing the proper reaction if yes.
 *
 * @param slave_id Value in interval [1,2] identifying the sensor chip.
 * @param channel_id Value in interval [1,4] identifying the channel to read
 *						data from.
 */
static msg_t get_value(int slave_id, int channel_id) {
	/**
	 * Array storing for each channel the number of data already acquire until
	 * the default_value has been updated.
	 */
	static int count[2][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}};

	/**
	 * The value read from the sensor.
	 */
	int value = 0;

	if (read_register(FDC_ADDR[slave_id - 1], DATA_MSB[channel_id]) != MSG_OK)
		return MSG_RESET;
	value = i2c_rx_buffer[0] << 24 | i2c_rx_buffer[1] << 16;

	if (read_register(FDC_ADDR[slave_id - 1], DATA_LSB[channel_id]) != MSG_OK)
		return MSG_RESET;
	value |= i2c_rx_buffer[0] << 8 | i2c_rx_buffer[1];

	add_value(0, value);
	if (count[slave_id - 1][channel_id] < BUFFER_SIZE)
		count[slave_id - 1][channel_id]++;
	else if (count[slave_id - 1][channel_id] == BUFFER_SIZE) {
		update_default_value(0);
		count[slave_id - 1][channel_id]++;
	} //else  count[slave_id][sensor_id] > BUFFER_SIZE
		//chprintf((BaseSequentialStream*)&RTTD,
		//		"Slave %i Sensor %i: %i\r\n",
		//		slave_id,
		//		channel_id,
		//		detect_action(0));
		// TODO react to touch detection.

	return MSG_OK;
}

static THD_WORKING_AREA(fdc_wa, FDC_WA_SIZE);
static THD_FUNCTION(fdc_int, arg) {
	UNUSED(arg);

	/**
	 * Return code of the I2C commands.
	 */
	i2cflags_t status;

	/**
	 * Address of the sensor that has awaken the thread.
	 */
	int addr;

	/**
	 * The channel that raised the interrupt.
	 */
	static msg_t sensor;

	chprintf((BaseSequentialStream*)&RTTD, "Start fdc thread\r\n");
	init_touch_detection(0);
	while(TRUE) {
		(void)chMBFetch(&sensors_box, &sensor, TIME_INFINITE);
		addr = FDC1_ADDR;
		if (sensor == 2)
			addr = FDC2_ADDR;
		status = read_register(addr, STATUS);
		if (status != I2C_NO_ERROR) {
			chprintf((BaseSequentialStream*)&RTTD, "error %i\r\n", (int)i2cGetErrors(&I2CD1));
			continue;
		}
		status = i2c_rx_buffer[0] << 8 | i2c_rx_buffer[1];
		if (status & DRDY && sensor > 0 && sensor < 3) {
			for (int i = 0; i < CHANNELS_NB[sensor - 1]; i++) {
				if (get_value(sensor, i) != MSG_OK)
					break;
			}
		}
	}
}

void fdc_init(void){
	/**
	 * Return code of the I2C commands.
	 */
	i2cflags_t status;

	i2c_set_pins();
	i2cStart(&I2CD1, &i2c1_cfg);
	chThdSleepMilliseconds(1000);
	do {
		chThdSleepMilliseconds(100);
		status = init_sensors();
	} while (status != I2C_NO_ERROR);
	chThdCreateStatic(fdc_wa, sizeof(fdc_wa), NORMALPRIO + 2, fdc_int, NULL);
	extStart(EXTD, &ext_config);
}

void fdc_cb (EXTDriver* driver, expchannel_t channel) {
	UNUSED(driver);
	chSysLockFromISR();
	if (channel == 10)
		chMBPostI(&sensors_box, (msg_t)1);
	else
		chMBPostI(&sensors_box, (msg_t)2);
	chSysUnlockFromISR();
}
