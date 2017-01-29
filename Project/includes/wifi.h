#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>
#include <stdbool.h>

/***********************/
/*        Defines      */
/***********************/

typedef enum error_code_t {NO_ERROR,
                            COMMAND_FAILED,
                            PARSE_ERROR,
                            UNKNOWN_COMMAND,
                            TOO_FEW_ARGS,
                            TOO_MANY_ARGS,
                            UNKNOWN_VARIABLE_OR_OPTION,
                            INVALID_ARGUMENT,
                            OVERFLOW_ERROR,
                            BOUNDS_ERROR,
                            HEADER_ERROR,
                            HEADER_TIMEOUT,
                            NO_DATA,
                            SAFEMODE} error_code_t;

/**
 * Aggregates all the information available in the command return codes.
 *
 * @var error : boolean value indicating if an error occured
 * @var error_code : the error code.
 * @var length : number of bytes actually sent by the Wi-Fi module in the
 *      following payload.
 */
typedef struct wifi_response_header {
    bool error;
    error_code_t error_code;
    int length;
} wifi_response_header;

/**
 * Aggregates all the information about a running connection.
 *
 * @var channel_id string containing the id of the channel allocated to this
 *      connection.
 */
typedef struct wifi_connection {
    char channel_id[3];
} wifi_connection;

/**
 * Size (in bytes) of the buffer that contains the headers (and size of the
 *      headers).
 */
#define WIFI_HEADER_SIZE 9

/**
 * Size (in bytes) of the buffer that contains the payload.
 */
#define WIFI_BUFFER_SIZE 1420

/***********************/
/*       Variables     */
/***********************/

/**
 * Buffer to store return codes of commands sent to the Wi-Fi module.
 */
extern char response_code[WIFI_HEADER_SIZE];

/**
 * Buffer to store the data returned by the commands sent to the module.
 */
extern char response_body[WIFI_BUFFER_SIZE];

/**
 * Base address of the server.
 */
extern const char* const address;

extern const char* const REBOOT;
extern const char* const NETWORK_FLUSH;
extern const char* const PING_CONN;

/***********************/
/*       Functions     */
/***********************/

/**
 * @brief Initialize the pins and the wifi connection.
 */
void wifiInit(void);

/**
 * Exported for tests only.
 */
wifi_response_header parse_response_code(void);

/** @brief Decode the input buffer as the channel id and store it in conn.
 *
 * @param conn Output variable where to store the id.
 */
void get_channel_id(wifi_connection* conn);



/** @brief Issue a read request to the Wi-Fi module.
 *
 * This command asks for as many data as required to fill the Wi-Fi input
 *      buffer, but less data can be actually read.
 *
 * @param conn The connection to read data from.
 * @param timeout Boolean value that indicates whether or not timeouts are
 *      required for blocking function calls.
 */
wifi_response_header read_buffer(wifi_connection conn, bool timeout);

/** @brief Issue a read request to the Wi-Fi module.
 *
 * @param conn The connection to read data from.
 * @param size The number of bytes to read (the actual number of bytes read can
 *  be lower). **WARNING**: This value must be lower than WIFI_BUFFER_SIZE - 2.
 * @param timeout Boolean value that indicates whether or not timeouts are
 *      required for blocking function calls.
 */
wifi_response_header read(wifi_connection conn, int size, bool timeout);

void read_music(char* path);

/** @brief Send a command to the Wi-Fi module.
 *
 * @param cmd The command to send.
 * @param timeout Boolean value that indicates whether or not timeouts are
 *      required for blocking function calls.
 */
wifi_response_header send_cmd(const char* cmd, bool timeout);

/** @brief Send data to the wifi module.
 *
 * @param conn The wifi connection to send data to.
 * @param length The number of bytes to send.
 * @param buffer The buffer to send data from.
 * @param timeout Boolean value that indicates whether or not timeouts are
 *      required for blocking function calls.
 */
wifi_response_header wifi_write(wifi_connection* conn, int length, uint8_t* buffer, bool timeout);

/**
 * @brief Reset the WiFi module in normal mode (from safe mode).
 */
int exit_safe_mode(void);

#endif
