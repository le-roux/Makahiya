#include "shell_user.h"
#include "utils.h"
#include "usbcfg.h"
#include "chprintf.h"
#include "stdlib.h"
#include <string.h>
#include "serial_user.h"
#include "wifi.h"
#include "sound.h"

thread_t* shelltp = NULL;
static const char* address = "http://makahiya.herokuapp.com";
static const char* get = "http_get ";

void servo(BaseSequentialStream* chp, int argc, char* argv[]) {
    if (argc != 1) {
        chprintf(chp, "Usage: servo angle [0-170]\r\n");
        return;
    }
    pwmEnableChannel(&PWMD1, 0, 70 + atoi(argv[0]));
}

void serial_start(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(chp);
    UNUSED(argc);
    UNUSED(argv);
    sdStart(wifi_SD, &serial_cfg);
}

void serial_stop(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(chp);
    UNUSED(argc);
    UNUSED(argv);
    sdStop(wifi_SD);
}

void send(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    int length = 0;
    for (int i = 0; i < argc; i++) {
        if (i != 0) {
            serial_tx_buffer[length] = ' ';
            serial_tx_buffer[length + 1] = '\0';
            length++;
        }
        length += strlen(argv[i]);
        if (i == 0)
            strcpy((char*)serial_tx_buffer, argv[i]);
        else
            strcat((char*)serial_tx_buffer, argv[i]);
    }
    serial_tx_buffer[length] = '\n';
    length++;
    sdWrite(wifi_SD, serial_tx_buffer, length);

    (void)get_response();
    chprintf(chp, response_body);
    wifi_connection conn;
    get_channel_id(response_body, &conn);
    read(conn);
    (void)get_response();
    chprintf(chp, response_body);
}

void read_music(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    int length = 0;

    // Prepare the request to send
    strcpy((char*)serial_tx_buffer, get);
    length += strlen(get);
    strcat((char*)serial_tx_buffer, address);
    length += strlen(address);
    strcat((char*)serial_tx_buffer, "/static/music.mp3\n");
    length += strlen("/static/music.mp3\n");

    for (int i = 0; i < length; i++)
        chSequentialStreamPut(chp, serial_tx_buffer[i]);
    chprintf(chp, "\r\n");

    // Actually send the request
    sdWrite(wifi_SD, serial_tx_buffer, length);

    // Read the response code
    wifi_response_header out = get_response();
    chprintf(chp, "get response\r\n");
    if (out.error == 1) {
        chprintf(chp, "Error (code: %i)\r\n", out.error_code);
        return;
    }
    else
        chprintf(chp, "Success (size: %i)\r\n", out.length);

    chprintf(chp, "Body: %s", response_body);
    get_channel_id(response_body, &audio_conn);

    /**
     * Read the content of the mp3 file.
     */

    chBSemSignal(&audio_bsem);
}

const ShellCommand commands[] = {
  {"servo", servo},
  {"serial_start", serial_start},
  {"serial_stop", serial_stop},
  {"send", send},
  {"read_music", read_music},
  {NULL, NULL}
};

const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1,
  commands
};
