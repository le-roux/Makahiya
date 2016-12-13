#ifndef WIFI_H
#define WIFI_H

/***********************/
/*        Defines      */
/***********************/

/**
 * Aggregates all the information available in the command return codes.
 */
typedef struct wifi_response_header {
    int error;
    int error_code;
    int length;
} wifi_response_header;

/**
 * Aggregates all the information about a running connection.
 */
typedef struct wifi_connection {
    char channel_id[3];
} wifi_connection;

#define WIFI_HEADER_SIZE 9
#define WIFI_BUFFER_SIZE 500

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

/***********************/
/*       Functions     */
/***********************/

/** @brief Decode the input buffer as the channel id and store it in conn.
 *
 * @param input A buffer containing the id allocated to the current connection
 *      by the Wi-Fi module.
 * @param conn Output variable where to store the id.
 */
void get_channel_id(char* input, wifi_connection* conn);

/** @brief Retrieve and decode the return code of a command.
 *
 * @return A structure containing all the information available in the
 *      code returned by the command previously sent to the Wi-Fi module.
 */
wifi_response_header get_response(void);

/** @brief Issue a read request to the Wi-Fi module.
 *
 * This command asks for as many data as required to fill the Wi-Fi input
 *      buffer, but less data can be actually read.
 *
 * @param conn The connection to read data from.
 */
void read(wifi_connection conn);

#endif
