#include "wifi.h"
#include "stdlib.h"
#include <ctype.h>
#include <string.h>

#ifndef TEST
#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "chprintf.h"
#include "serial_user.h"
#include "utils.h"
#include "sound.h"
#endif // TEST

/***********************/
/*       Variables     */
/***********************/
char response_code[WIFI_HEADER_SIZE];
char response_body[WIFI_BUFFER_SIZE];
wifi_connection audio_conn;
static const char* const read_cmd = "read ";
const char* address = "http://makahiya.herokuapp.com";
const char* get = "http_get ";
static MUTEX_DECL(serial_mutex);
#define MEASURE_TIME 0

/***********************/
/*       Functions     */
/***********************/
wifi_response_header parse_response_code(void) {
    wifi_response_header out;
    if (response_code[0] != 'R') {
        out.error = 1;
        out.error_code = HEADER_ERROR;
        out.length = 0;
        return out;
    }
    out.error = response_code[1] != '0';
    out.error_code = response_code[1] - '0';
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
wifi_response_header get_response(int timeout) {
    static int end;
    wifi_response_header out;

    #if MEASURE_TIME
    systime_t start = chVTGetSystemTime();
    #endif

    int res;

    chMtxLock(&serial_mutex);
    if (timeout)
        res = sdReadTimeout(wifi_SD, (uint8_t*)response_code, WIFI_HEADER_SIZE, MS2ST(400));
    else
        res = sdRead(wifi_SD, (uint8_t*)response_code, WIFI_HEADER_SIZE);
    chMtxUnlock(&serial_mutex);

    #if MEASURE_TIME
    DEBUG("header laps -> %d", ST2MS(chVTTimeElapsedSinceX(start)));
    #endif

    if (res != WIFI_HEADER_SIZE) {
        out.error = 1;
        out.error_code = HEADER_TIMEOUT;
        DEBUG("Error in header (res: %i)", res);
        return out;
    }

    out = parse_response_code();
    if (out.error_code == HEADER_ERROR)
        return out; // No recovery solution

    // Trying to read 0 byte rises an error, so a check is needed.
    if (out.length > 0) {
        end = 0;

        #if MEASURE_TIME
        start = chVTGetSystemTime();
        #endif

        chMtxLock(&serial_mutex);
        if (timeout)
            res = sdReadTimeout(wifi_SD, (uint8_t*)response_body, out.length, MS2ST(80));
        else
            res = sdRead(wifi_SD, (uint8_t*)response_body, out.length);
        chMtxUnlock(&serial_mutex);

        #if MEASURE_TIME
        DEBUG("body laps -> %d", ST2MS(chVTTimeElapsedSinceX(start)));
        #endif

        if (res != out.length)
            DEBUG("length %i (expected %i)", res, out.length);
    } else {
        if (!end)
            end = 1;
        else {
            out.error = 1;
            out.error_code = NO_DATA;
        }
        chprintf((BaseSequentialStream*)&SDU1, "Nothing to read\r\n");
    }
    response_body[out.length] = '\0';
    return out;
}

void read_buffer(wifi_connection conn) {
    read(conn, WIFI_BUFFER_SIZE - 2);
}

void read(wifi_connection conn, int size) {
    chDbgCheck(size <= WIFI_BUFFER_SIZE - 2);

    int length = 0;
    // Prepare the read request.
    strcpy((char*)serial_tx_buffer, read_cmd);
    length += strlen(read_cmd);
    strcat((char*)serial_tx_buffer, conn.channel_id);
    length += strlen(conn.channel_id);
    strcat((char*)serial_tx_buffer, " ");
    length += strlen(" ");
    char tmp[5];
    int_to_char(tmp, size);
    strcat((char*)serial_tx_buffer, tmp);
    length += strlen(tmp);
    strcat((char*)serial_tx_buffer, "\n\r");
    length += strlen("\n\r");

    // Actually send the request.
    chMtxLock(&serial_mutex);
    sdWrite(wifi_SD, serial_tx_buffer, length);
    chMtxUnlock(&serial_mutex);
}

void read_music(char* path) {
    int length = 0;

    // Prepare the request to send
    strcpy((char*)serial_tx_buffer, get);
    length += strlen(get);
    strcat((char*)serial_tx_buffer, address);
    length += strlen(address);
    strcat((char*)serial_tx_buffer, path);
    length += strlen(path);
    strcat((char*)serial_tx_buffer, "\n");
    length += strlen("\n");

    // Actually send the request
    chMtxLock(&serial_mutex);
    sdWrite(wifi_SD, serial_tx_buffer, length);
    chMtxUnlock(&serial_mutex);

    // Read the response code
    wifi_response_header out = get_response(true);
    if (out.error == 1) {
        DEBUG("Error (code: %i)\r\n", out.error_code);
        DEBUG("Body: %s", response_body);
        return;
    } else
        DEBUG("Success (size: %i)\r\nBody: %s", out.length,response_body);

    get_channel_id(&audio_conn);

    /**
     * Start the reading of the mp3 file.
     */
    chBSemSignal(&audio_bsem);
}

void send_cmd(char* cmd) {
    chDbgCheck(strlen(cmd) < SERIAL_TX_BUFFER_SIZE - 1);

    int length = 0;
    strcpy((char*)serial_tx_buffer, cmd);
    length += strlen(cmd);
    strcat((char*)serial_tx_buffer, "\n");
    length += strlen("\n");

    chMtxLock(&serial_mutex);
    SEND_DATA(serial_tx_buffer, length);
    chMtxUnlock(&serial_mutex);
}

void wifi_write(wifi_connection* conn, int length, uint8_t* buffer) {
    char cmd[12], channel[5];
    strcpy(cmd, "write ");
    strcat(cmd, conn->channel_id);
    strcat(cmd, " ");
    int_to_char(channel, length);
    strcat(cmd, channel);
    send_cmd(cmd);
    SEND_DATA(buffer, length);
}

void clear_body(void) {
    for (int i = 0; i < WIFI_BUFFER_SIZE; i++)
        response_body[i] = '\0';
}

#endif // TEST
