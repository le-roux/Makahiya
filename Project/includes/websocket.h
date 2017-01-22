#ifndef WEBSOCKET_H
#define WEBSOCKET_H

/***********************/
/*        Defines      */
/***********************/

/**
 * Memory space (in bytes) allocated to the thread handling the websocket.
 */
#define WA_WEBSOCKET_SIZE 256

/**
 * Possible values for the return code of the "poll" command.
 */

/**
 * Connection opened but no data to read.
 */
#define EMPTY '0'

/**
 * Connection opened and incoming data to read.
 */
#define DATA_AVAILABLE '1'

/**
 * Connection remotely closed.
 */
#define CONN_CLOSED '2'

/***********************/
/*       Variables     */
/***********************/

extern THD_WORKING_AREA(wa_websocket, WA_WEBSOCKET_SIZE);

/***********************/
/*       Functions     */
/***********************/

/**
 * @brief Callback called when the Wi-Fi module raises an interrupt.
 */
void web_cb(EXTDriver* driver, expchannel_t channel);

/** @brief Main function of the websocket thread.
 *
 * @param arg A string indicating the plant id (unique for each plant).
 */
extern THD_FUNCTION(websocket, arg);

#endif
