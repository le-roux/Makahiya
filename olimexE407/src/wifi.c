#include "wifi.h"
#include "stdlib.h"
#include <ctype.h>

#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "chprintf.h"
#include "serial_user.h"
#include <string.h>

/***********************/
/*       Variables     */
/***********************/
char response_code[WIFI_HEADER_SIZE];
char response_body[WIFI_BUFFER_SIZE];
static const char* read_cmd = "read ";

/***********************/
/*       Functions     */
/***********************/
static wifi_response_header parse_response_code(void) {
    wifi_response_header out;
    for (int i = 0; i < 7; i++)
        chSequentialStreamPut((BaseSequentialStream*)&SDU1, response_code[i]);
    chprintf((BaseSequentialStream*)&SDU1, "\r\n");
    if (response_code[0] != 'R') {
        out.error = 1;
        out.error_code = -1;
        return out;
    }
    if (response_code[1] != '0')
        out.error = 1;
    else
        out.error = 0;
    out.error_code = response_code[1] - '0';
    out.length = atoi(&response_code[2]);
    return out;
}

void get_channel_id(char* response_body, wifi_connection* conn) {
    int i = 0;
    while (isdigit(response_body[i])) {
        conn->channel_id[i] = response_body[i];
        i++;
    }
    conn->channel_id[i] = '\0';
}

wifi_response_header get_response(void) {
    sdRead(wifi_SD, (uint8_t*)response_code, WIFI_HEADER_SIZE);
    wifi_response_header out = parse_response_code();
    if (out.error & (out.error_code == -1))
        return out;
    sdRead(wifi_SD, (uint8_t*)response_body, out.length);
    response_body[out.length] = '\0';
    return out;
}

void read(wifi_connection conn) {
    int length = 0;
    char tmp[4];

    // Prepare the read request.
    strcpy((char*)serial_tx_buffer, read_cmd);
    length += strlen(read_cmd);
    strcat((char*)serial_tx_buffer, conn.channel_id);
    length += strlen(conn.channel_id);
    strcat((char*)serial_tx_buffer, " ");
    length += strlen(" ");
    itoa(WIFI_BUFFER_SIZE - 2, tmp, 10);
    strcat((char*)serial_tx_buffer, tmp);
    length += strlen(tmp);
    strcat((char*)serial_tx_buffer, "\n");
    length += strlen("\n");

    // Actually send the request.
    sdWrite(wifi_SD, serial_tx_buffer, length);
}
