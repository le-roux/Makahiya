#ifndef FDC2214_H
#define FDC2214_H

#include <stdint.h>
#include "hal.h"

/************************/
/*       Defines        */
/************************/
#define FDC1_ADDR 0x2A
#define FDC2_ADDR 0x2B

#define DATA_MSB_CH0 0x00
#define DATA_LSB_CH0 0x01

#define DATA_MSB_CH1 0x02
#define DATA_LSB_CH1 0x03

#define DATA_MSB_CH2 0x04
#define DATA_LSB_CH2 0x05

#define DATA_MSB_CH3 0x06
#define DATA_LSB_CH3 0x07

#define RCOUNT_CH0 0x08
#define RCOUNT_CH1 0x09
#define RCOUNT_CH2 0x0A
#define RCOUNT_CH3 0x0B

#define OFFSET_CH0 0x0C
#define OFFSET_CH1 0x0D
#define OFFSET_CH2 0x0E
#define OFFSET_CH3 0x0F

#define SETTLECOUNT_CH0 0x10
#define SETTLECOUNT_CH1 0x11
#define SETTLECOUNT_CH2 0x12
#define SETTLECOUNT_CH3 0x13

#define CLOCK_DIVIDERS_0 0x14
#define CLOCK_DIVIDERS_1 0x15
#define CLOCK_DIVIDERS_2 0x16
#define CLOCK_DIVIDERS_3 0x17

#define STATUS 0x18
#define STATUS_CONFIG 0x19
#define CONFIG 0x1A
#define MUX_CONFIG 0x1B
#define RESET_DEV 0x1C

#define DRIVE_CURRENT_CH0 0x1E
#define DRIVE_CURRENT_CH1 0x1F
#define DRIVE_CURRENT_CH2 0x20
#define DRIVE_CURRENT_CH3 0x21

#define MANUFACTURER_ID 0x7E
#define DEVICE_ID 0x7F

/**
 * Config register components
 */
#define SLEEP_MODE 0x2000
#define SENSOR_ACTIVATE_SEL 0x0800
#define REF_CLK_SRC 0x0200
#define CONFIG_RESERVED 0x1401

#define DRDY 0x40

#define FDC_WA_SIZE 2048

/*************************/
/*      Macros        */
/*************************/
#define ENTER_SLEEP_MODE config &= SLEEP_MODE
#define RESET_CONFIG config = CONFIG_RESERVED

/*************************/
/*      Variables        */
/*************************/
extern uint16_t config;
extern uint16_t status;
extern binary_semaphore_t fdc_bsem;
extern int FDC_ADDR[2];
extern int DATA_MSB[4];
extern int DATA_LSB[4];
extern volatile uint8_t calling;

/*************************/
/*      Functions        */
/*************************/

void fdc_init(void);

#endif // FDC2214_H
