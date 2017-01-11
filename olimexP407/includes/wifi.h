#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>

/***********************/
/*        Defines      */
/***********************/

/**
 * Aggregates all the information available in the command return codes.
 *
 * @var error : boolean value indicating if an error occured
 * @var error_code : the error code. Possible values are:
 *      - -1: incorrect header received.
 *      - -2:
 *      - -3: header not fully received.
 *      - -4: nothing to read two sequential times -> stop the audio.
 * @var length : number of bytes actually sent by the Wi-Fi module in the
 *      following payload.
 */
typedef struct wifi_response_header {
    int error;
    int error_code;
    int length;
} wifi_response_header;

#define HEADER_ERROR   -1

#define HEADER_TIMEOUT -3
#define NO_DATA        -4

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

#define SEND_DATA(data, length) sdWrite(wifi_SD, data, length)

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
 * Variable to store the channel id of the audio download connection.
 */
extern wifi_connection audio_conn;

extern const char* address;
extern const char* get;

/***********************/
/*       Functions     */
/***********************/

/**
 * Exported for tests only.
 */
wifi_response_header parse_response_code(void);

/** @brief Decode the input buffer as the channel id and store it in conn.
 *
 * @param conn Output variable where to store the id.
 */
void get_channel_id(wifi_connection* conn);

/** @brief Retrieve and decode the return code of a command.
 *
 * @param timeout Boolean value that indicates whether or not timeouts are
 *      required for blocking function calls.
 *
 * @return A structure containing all the information available in the
 *      code returned by the command previously sent to the Wi-Fi module.
 */
wifi_response_header get_response(int timeout);

/** @brief Issue a read request to the Wi-Fi module.
 *
 * This command asks for as many data as required to fill the Wi-Fi input
 *      buffer, but less data can be actually read.
 *
 * @param conn The connection to read data from.
 */
void read_buffer(wifi_connection conn);

/** @brief Issue a read request to the Wi-Fi module.
 *
 * @param conn The connection to read data from.
 * @param size The number of bytes to read (the actual number of bytes read can
 *  be lower). **WARNING**: This value must be lower than WIFI_BUFFER_SIZE - 2.
 */
void read(wifi_connection conn, int size);

void read_music(char* path);

/** @brief Send a command to the Wi-Fi module.
 *
 */
void send_cmd(char* cmd);

void wifi_write(wifi_connection* conn, int length, uint8_t* buffer);

void clear_body(void);

#endif
