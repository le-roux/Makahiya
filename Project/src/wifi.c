#include "wifi.h"
#include "stdlib.h"
#include <ctype.h>
#include <string.h>

#ifndef TEST
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "serial_user.h"
#include "utils.h"
#include "sound.h"
#include "RTT_streams.h"
#endif // TEST

/***********************/
/*       Variables     */
/***********************/
char response_code[WIFI_HEADER_SIZE];
char response_body[WIFI_BUFFER_SIZE];

const char* const address = "http://makahiya.rfc1149.net";
const char* const REBOOT = "reboot";
const char* const NETWORK_FLUSH = "network_flush";
const char* const PING_CONN = "ping -g";

#ifndef TEST
MUTEX_DECL(wifi_mutex);
#endif // TEST
#define SEND_DATA(data, length) sdWrite(wifi_SD, data, length)
#define SEND_DATA_TIMEOUT(data, length, timeout) sdWriteTimeout(wifi_SD, data, \
                                                            length, timeout)

/***********************/
/*       Functions     */
/***********************/


wifi_response_header parse_response_code(void) {
    wifi_response_header out;
    if (response_code[0] == 'R') {
        out.error = (response_code[1] != '0');
        out.error_code = response_code[1] - '0';
    } else if (response_code[0] == 'S') {
        out.error = true;
        out.error_code = SAFEMODE;
    } else {
        out.error = true;
        out.error_code = HEADER_ERROR;
        out.length = 0;
        return out;
    }
    out.length = atoi(&response_code[2]);
    return out;
}

void get_channel_id(wifi_connection* conn) {
    int i = 0;
    while (isdigit(response_body[i])) {
        conn->channel_id[i] = response_body[i];
        i++;
    }
    conn->channel_id[i] = '\0';
}

#ifndef TEST

/** @brief Retrieve and decode the return code of a command.
 *
 * @param dest A pointer to the buffer that will receive the data. This buffer
 *              has to be large enough to hold all the data.
 * @param timeout Boolean value that indicates whether or not timeouts are
 *      required for blocking function calls.
 * @param string Boolean value that indicates whether or not the response body will
 *              be treated as a string.
 *
 * @return A structure containing all the information available in the
 *      code returned by the command previously sent to the Wi-Fi module.
 */
static wifi_response_header get_response(uint8_t* dest, bool timeout, bool string) {
    /**
     * Counter used to handle the NO_DATA error.
     */
    static int end = 0;

    /**
     * Duration in milliseconds of the timeout for the reading functions.
     */
    static int timing = 40;

    /**
     * Content of the response header. It's the value returned by the function.
     */
    wifi_response_header out;

    /**
     * Return value of the sending functions.
     */
    int res;

    /**
     * Memory space that will receive the END_OF_FRAME characters.
     */
    static uint8_t trash[2];

    if (timeout)
        res = sdReadTimeout(wifi_SD, (uint8_t*)response_code, WIFI_HEADER_SIZE, MS2ST(timing));
    else
        res = sdRead(wifi_SD, (uint8_t*)response_code, WIFI_HEADER_SIZE);

    if (res != WIFI_HEADER_SIZE) {
        out.error = true;
        out.error_code = HEADER_TIMEOUT;
        return out;
    }

    out = parse_response_code();
    if (out.error_code == HEADER_ERROR)
        return out; // No recovery solution

    // Trying to read 0 byte rises an error, so a check is needed.
    if (out.length > 0) {
        end = 0;

        out.length -= 2; // Remove the 'END OF FRAME' characters
        if (timeout) {
            res = sdReadTimeout(wifi_SD, dest, out.length, MS2ST(timing));
            res = sdReadTimeout(wifi_SD, trash, 2, MS2ST(timing));
        } else {
            res = sdRead(wifi_SD, dest, out.length);
            res = sdRead(wifi_SD, trash, 2);
        }
    } else { // Nothing to read.
        if (end < 20) {
            end++;
        } else {
            out.error = true;
            out.error_code = NO_DATA;
        }
    }
    if (string)
        response_body[out.length] = '\0'; // Allow this buffer to be used as a string.
    return out;
}

static void wifi_set_pins(void) {
    // Interrupt for websockets
    palSetPadMode(GPIOA, 11, PAL_MODE_INPUT_PULLDOWN);

    // ResetN
    palSetPadMode(GPIOA, 15, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPad(GPIOA, 15);
}

void wifiInit(void) {
    /**
    * Header of the response sent by the Wi-Fi module.
    */
    wifi_response_header out;

    serial_set_pin();
    wifi_set_pins();
    sdStart(wifi_SD, &serial_config);
    do {
        DEBUG("Trying to connect to WiFi network");
        out = send_cmd(PING_CONN, false);
        if (out.error && out.error_code == SAFEMODE)
            exit_safe_mode();
        chThdSleepMilliseconds(100);
    } while(out.error);
    DEBUG("wifi OK");
}

wifi_response_header read_buffer(wifi_connection conn, bool timeout) {
    return wifi_read(conn, (uint8_t*)response_body, WIFI_BUFFER_SIZE - 2, timeout, true);
}

wifi_response_header wifi_read(wifi_connection conn, uint8_t* dest, int size, bool timeout, bool string) {

    /**
     * Header of the response sent by the Wi-Fi module.
     */
    wifi_response_header header;

    /**
     * Buffer to store the string representation of @p size.
     */
    char tmp[10];

    int_to_char(tmp, size);

    chMtxLock(&wifi_mutex);
    // Prepare the read request.
    strcpy((char*)serial_tx_buffer, "read ");
    strcat((char*)serial_tx_buffer, conn.channel_id);
    strcat((char*)serial_tx_buffer, " ");
    strcat((char*)serial_tx_buffer, tmp);
    strcat((char*)serial_tx_buffer, "\r\n");

    // Actually send the request.
    SEND_DATA(serial_tx_buffer, strlen((char*)serial_tx_buffer));
    header = get_response(dest, timeout, string);
    chMtxUnlock(&wifi_mutex);
    return header;
}

void read_music(char* path) {

    /**
     * Header of the response sent by the Wi-Fi module.
     */
    wifi_response_header header;

    chMtxLock(&wifi_mutex);
    // Prepare the request to send
    strcpy((char*)serial_tx_buffer, "http_get ");
    strcat((char*)serial_tx_buffer, address);
    strcat((char*)serial_tx_buffer, path);
    strcat((char*)serial_tx_buffer, "\r\n");

    // Actually send the request
    SEND_DATA(serial_tx_buffer, strlen((char*)serial_tx_buffer));

    // Read the response code
    header = get_response((uint8_t*)response_body, false, true);
    DEBUG("res %s %s", response_code, response_body);
    if (header.error == 1)
        return;

    get_channel_id((wifi_connection*)&audio_conn);
    chMtxUnlock(&wifi_mutex);

    /**
     * Start the reading of the mp3 file.
     */
    chBSemSignal(&download_bsem);
}

wifi_response_header send_cmd(const char* cmd, bool timeout) {
    chDbgCheck(strlen(cmd) < SERIAL_TX_BUFFER_SIZE - 1);

    /**
     * Header of the response sent by the Wi-Fi module.
     */
    wifi_response_header header;

    /**
    * Number of bytes to send.
    */
    int length;

    /**
     * Number of bytes actually sent.
     */
    int sent;

    chMtxLock(&wifi_mutex);
    strcpy((char*)serial_tx_buffer, cmd);
    strcat((char*)serial_tx_buffer, "\r\n");
    length = strlen((char*)serial_tx_buffer);

    do {
        sent = SEND_DATA_TIMEOUT(serial_tx_buffer, length, MS2ST(100));
    } while (sent != length);
    header = get_response((uint8_t*)response_body, timeout, true);
    chMtxUnlock(&wifi_mutex);
    return header;
}

wifi_response_header wifi_write(wifi_connection* conn, int length, uint8_t* buffer, bool timeout) {

    /**
     * Header of the response sent by the Wi-Fi module.
     */
    wifi_response_header header;

    /**
     * Buffer to store the string representation of the number of bytes to send.
     */
    char size[10];

    chMtxLock(&wifi_mutex);

    strcpy((char*)serial_tx_buffer, "write ");
    strcat((char*)serial_tx_buffer, conn->channel_id);
    strcat((char*)serial_tx_buffer, " ");
    int_to_char(size, length);
    strcat((char*)serial_tx_buffer, size);
    strcat((char*)serial_tx_buffer, "\r\n");

    SEND_DATA(serial_tx_buffer, strlen((char*)serial_tx_buffer));
    SEND_DATA(buffer, length);

    header = get_response((uint8_t*)response_body, timeout, true);

    chMtxUnlock(&wifi_mutex);
    return header;
}

int exit_safe_mode(void) {

    /**
     * Header of the response sent by the Wi-Fi module.
     */
    wifi_response_header header;

    header = send_cmd("get system.safemode.status", false);
    if (header.error && header.error_code == SAFEMODE) {
        header = send_cmd("faults_reset", false);
        header = send_cmd(REBOOT, false);
        return 0;
    }
    return 1;
}

#endif // TEST
