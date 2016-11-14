#ifndef EXT_USER_H
#define EXT_USER_H

#include "hal.h"

/** @brief Init all the user-defined variables and start the proper EXT driver.
 *
 */
void ext_init(void);

/**
 * Increase or decrease the brightness of the STAT1 led when the WAKEUP button is
 * clicked.
 */
void ext_cb1(EXTDriver* driver, expchannel_t channel);

/**
 * Increase or decrease the brightness of the STAT2 led when the TAMPER button is
 * clicked.
 */
void ext_cb2(EXTDriver* driver, expchannel_t channel);

/** @brief Suspend time for anti-rebounce.
 *
 * The interrupt managing the led intensity is ignored for this time after each
 * processed change.
 */
#define SUSPEND_TIME 500

#endif // EXT_USER_H
