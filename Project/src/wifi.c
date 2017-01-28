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
        out.error = 1;
        out.error_code = SAFEMODE;
    } else {
        out.error = 1;
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
 * @param timeout Boolean value that indicates whether or not timeouts are
 *      required for blocking function calls.
 *
 * @return A structure containing all the information available in the
 *      code returned by the command previously sent to the Wi-Fi module.
 */
static wifi_response_header get_response(int timeout) {
    static int end, timing = 40;
    wifi_response_header out;

    int res;

    if (timeout)
        res = sdReadTimeout(wifi_SD, (uint8_t*)response_code, WIFI_HEADER_SIZE, MS2ST(timing));
    else
        res = sdRead(wifi_SD, (uint8_t*)response_code, WIFI_HEADER_SIZE);

    if (res != WIFI_HEADER_SIZE) {
        out.error = 1;
        out.error_code = HEADER_TIMEOUT;
        timing *= 2;
        return out;
    }

    out = parse_response_code();
    if (out.error_code == HEADER_ERROR)
        return out; // No recovery solution

    // Trying to read 0 byte rises an error, so a check is needed.
    if (out.length > 0) {
        end = 0;

        if (timeout)
            res = sdReadTimeout(wifi_SD, (uint8_t*)response_body, out.length, MS2ST(5));
        else
            res = sdRead(wifi_SD, (uint8_t*)response_body, out.length);
    } else {
        if (end < 20)
            end++;
        else {
            out.error = 1;
            out.error_code = NO_DATA;
        }
    }
    out.length -= 2; // Remove the 'END OF FRAME' characters
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

wifi_response_header read_buffer(wifi_connection conn, int timeout) {
    return read(conn, WIFI_BUFFER_SIZE - 2, timeout);
}

wifi_response_header read(wifi_connection conn, int size, int timeout) {
    chDbgCheck(size <= WIFI_BUFFER_SIZE - 2);

    wifi_response_header header;
    int length = 0;
    chMtxLock(&wifi_mutex);
    // Prepare the read request.
    strcpy((char*)serial_tx_buffer, "read ");
    length += strlen("read ");
    strcat((char*)serial_tx_buffer, conn.channel_id);
    length += strlen(conn.channel_id);
    strcat((char*)serial_tx_buffer, " ");
    length += strlen(" ");
    char tmp[5];
    int_to_char(tmp, size);
    strcat((char*)serial_tx_buffer, tmp);
    length += strlen(tmp);
    strcat((char*)serial_tx_buffer, "\r\n");
    length += strlen("\r\n");

    // Actually send the request.
    SEND_DATA(serial_tx_buffer, length);
    header = get_response(timeout);
    chMtxUnlock(&wifi_mutex);
    return header;
}

void read_music(char* path) {
    int length = 0;

    chMtxLock(&wifi_mutex);
    // Prepare the request to send
    strcpy((char*)serial_tx_buffer, "http_get ");
    length += strlen("http_get ");
    strcat((char*)serial_tx_buffer, address);
    length += strlen(address);
    strcat((char*)serial_tx_buffer, path);
    length += strlen(path);
    strcat((char*)serial_tx_buffer, "\r\n");
    length += strlen("\r\n");

    // Actually send the request
    SEND_DATA(serial_tx_buffer, length);

    // Read the response code
    wifi_response_header out = get_response(false);
    if (out.error == 1)
        return;

    get_channel_id((wifi_connection*)&audio_conn);
    chMtxUnlock(&wifi_mutex);

    /**
     * Start the reading of the mp3 file.
     */
    chBSemSignal(&download_bsem);
}

wifi_response_header send_cmd(const char* cmd, int timeout) {
    chDbgCheck(strlen(cmd) < SERIAL_TX_BUFFER_SIZE - 1);

    int length = 0, sent;
    wifi_response_header header;
    chMtxLock(&wifi_mutex);
    strcpy((char*)serial_tx_buffer, cmd);
    length += strlen(cmd);
    strcat((char*)serial_tx_buffer, "\r\n");
    length += strlen("\r\n");

    do {
        sent = SEND_DATA_TIMEOUT(serial_tx_buffer, length, MS2ST(100));
    } while (sent != length);
    header = get_response(timeout);
    chMtxUnlock(&wifi_mutex);
    return header;
}

wifi_response_header wifi_write(wifi_connection* conn, int length, uint8_t* buffer, int timeout) {
    char channel[5];
    wifi_response_header header;
    chMtxLock(&wifi_mutex);
    strcpy((char*)serial_tx_buffer, "write ");
    strcat((char*)serial_tx_buffer, conn->channel_id);
    strcat((char*)serial_tx_buffer, " ");
    int_to_char(channel, length);
    strcat((char*)serial_tx_buffer, channel);
    strcat((char*)serial_tx_buffer, "\r\n");
    SEND_DATA(serial_tx_buffer, strlen((char*)serial_tx_buffer));
    SEND_DATA(buffer, length);
    header = get_response(timeout);
    chMtxUnlock(&wifi_mutex);
    return header;
}

int exit_safe_mode(void) {
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
