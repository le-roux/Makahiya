#ifndef FDC2214_H
#define FDC2214_H

#include <stdint.h>
#include "hal.h"

/************************/
/*       Defines        */
/************************/
#define CONFIG_REG_ADDR 0x1A

/**
 * Config register components
 */
#define SLEEP_MODE 0x2000
#define SENSOR_ACTIVATE_SEL 0x0800
#define REF_CLK_SRC 0x0200
#define CONFIG_RESERVED 0x1401

/*************************/
/*      Macros        */
/*************************/
#define ENTER_SLEEP_MODE config &= SLEEP_MODE
#define RESET_CONFIG config = CONFIG_RESERVED

/*************************/
/*      Variables        */
/*************************/
extern uint16_t config;

/*************************/
/*      Functions        */
/*************************/
extern msg_t write_register(uint8_t addr, uint8_t reg_addr, uint16_t value);
extern msg_t read_register(uint8_t addr, uint8_t reg_addr);

#endif // FDC2214_H
