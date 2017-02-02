#include "fdc2214.h"
#include "i2c_user.h"
#include "ext_user.h"
#include "utils.h"
#include "capacitive_sensor.h"
#include "pwmdriver.h"

#include "chprintf.h"
#include "RTT_streams.h"

volatile uint8_t calling;
static int FDC_ADDR[2] = {FDC1_ADDR, FDC2_ADDR};
static int DATA_MSB[4] = {DATA_MSB_CH0, DATA_MSB_CH1, DATA_MSB_CH2, DATA_MSB_CH3};
static int DATA_LSB[4] = {DATA_LSB_CH0, DATA_LSB_CH1, DATA_LSB_CH2, DATA_LSB_CH3};
static int CHANNELS_NB[2] = {4, 4};

/**
 * @brief Buffer for the @p sensors_box mailbox.
 */
static msg_t sensors[2];

/**
 * @brief Mailbox used by the fdc thread to know which sensor has new data.
 */
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
	/**
	 * @brief Return code of the I2C commands.
	 */
	msg_t status;

	// Set the sensor in SLEEP_MODE
	status = write_register(FDC1_ADDR, CONFIG, CONFIG_RESERVED | SLEEP_MODE);
	if (status != MSG_OK)
		return i2cGetErrors(&I2CD1);

	// Set the clock divider registers
	status = write_register(FDC1_ADDR, CLOCK_DIVIDERS_0, 0x2001);
	status = write_register(FDC1_ADDR, CLOCK_DIVIDERS_1, 0x2001);
	status = write_register(FDC1_ADDR, CLOCK_DIVIDERS_2, 0x2001);
	status = write_register(FDC1_ADDR, CLOCK_DIVIDERS_3, 0x2001);

	// Set the drive current registers
	status = write_register(FDC1_ADDR, DRIVE_CURRENT_CH0, 0x8800);
	status = write_register(FDC1_ADDR, DRIVE_CURRENT_CH1, 0x8800);
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
 * @brief Acquire data from one channel of one sensor and update it in the
 *		algorithm.
 *
 * @param slave_id Value in interval [0,1] identifying the sensor chip.
 * @param channel_id Value in interval [0,3] identifying the channel to read
 *						data from.
 */
static int acquire_value(int slave_id, int channel_id) {

	/**
	 * The value read from the sensor.
	 */
	int value = 0;

	if (read_register(FDC_ADDR[slave_id], DATA_MSB[channel_id]) != MSG_OK)
		return -1;
	value = i2c_rx_buffer[0] << 24 | i2c_rx_buffer[1] << 16;

	if (read_register(FDC_ADDR[slave_id], DATA_LSB[channel_id]) != MSG_OK)
		return -1;
	value |= i2c_rx_buffer[0] << 8 | i2c_rx_buffer[1];
	if(slave_id == 0 && channel_id == 3)
		DEBUG("%i,", value);

	add_value(slave_id, channel_id, value);

	return 0;
}

static THD_WORKING_AREA(fdc_wa, FDC_WA_SIZE);
static THD_FUNCTION(fdc_int, arg) {
	UNUSED(arg);

	/**
	 * Return code of the I2C commands.
	 */
	i2cflags_t status;

	/**
	 * Content of the STATUS register of the sensor.
	 */
	uint16_t sensor_status;

	/**
	 * Address of the sensor that has awaken the thread.
	 */
	int addr;

	/**
	 * The sensor that raised the interrupt.
	 */
	static msg_t sensor;

	/**
	 * The action detected by the touch-detection algorithm.
	 */
	int action;

	//chprintf((BaseSequentialStream*)&RTTD, "Start fdc thread\r\n");

	for (int i = 0; i < 4; i++) {
		init_touch_detection(0, i);
		init_touch_detection(1, i);
	}

	while(TRUE) {
		(void)chMBFetch(&sensors_box, &sensor, TIME_INFINITE);
		if (sensor < 0 || sensor > 1) // Invalid value
			continue;
		addr = FDC1_ADDR;
		if (sensor == 1)
			addr = FDC2_ADDR;
		status = read_register(addr, STATUS);
		if (status != I2C_NO_ERROR) { // Error in the communication.
			//DEBUG("error %i\r\n", (int)i2cGetErrors(&I2CD1));
			continue;
		}
		sensor_status = i2c_rx_buffer[0] << 8 | i2c_rx_buffer[1];
		if (sensor_status & DRDY) { // Data ready to be read.
			for (int channel_id = 0; channel_id < CHANNELS_NB[sensor]; channel_id++) {
				acquire_value(sensor, channel_id);
				if (sensor == 0 && channel_id == 3) {
					action = detect_action(sensor, channel_id);
					if (action == 1)
						setLed(LED2_G, 12);
					else if (action == -1)
						setLed(LED2_G, 0);
				}
			}
		}
	}
}

void fdcInit(void){
	/**
	 * Return code of the I2C commands.
	 */
	i2cflags_t status;

	i2c_set_pins();
	i2cStart(&I2CD1, &i2c1_cfg);
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
		chMBPostI(&sensors_box, (msg_t)0);
	else
		chMBPostI(&sensors_box, (msg_t)1);
	chSysUnlockFromISR();
}
