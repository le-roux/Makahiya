#include "wifi.h"
#include "stdlib.h"
#include <ctype.h>

#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "chprintf.h"

/***********************/
/*       Variables     */
/***********************/
char response_code[WIFI_HEADER_SIZE];
char response_body[WIFI_BUFFER_SIZE];

/***********************/
/*       Functions     */
/***********************/

wifi_response parse_response_code(void) {
    wifi_response out;
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

void get_channel_id(char* response_body, char* out) {
    int i = 0;
    while (isdigit(response_body[i])) {
        out[i] = response_body[i];
        i++;
    }
    out[i] = '\0';
}
