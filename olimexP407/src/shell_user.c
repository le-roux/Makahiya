#include "shell_user.h"
#include "utils.h"
#include "usbcfg.h"
#include "chprintf.h"
#include "stdlib.h"
#include <string.h>
#include "serial_user.h"
#include "wifi.h"

thread_t* shelltp = NULL;
RTTStream rtt_str;

void servo(BaseSequentialStream* chp, int argc, char* argv[]) {
    if (argc != 1) {
        chprintf(chp, "Usage: servo angle [0-170]\r\n");
        return;
    }
    pwmEnableChannel(&PWMD1, 0, 70 + atoi(argv[0]));
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

    (void)get_response(false);
    chprintf(chp, response_body);
    wifi_connection conn;
    get_channel_id(&conn);
    read_buffer(conn);
    (void)get_response(false);
    chprintf(chp, response_body);
}

void read_music_shell(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(chp);
    UNUSED(argc);
    UNUSED(argv);

    read_music("/static/music.mp3");
}

void foo(BaseSequentialStream *chp, int argc, char* argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    chprintf(chp, "bar\r\n");
}

const ShellCommand commands[] = {
  {"servo", servo},
  {"send", send},
  {"read_music", read_music_shell},
  {"foo", foo},
  {NULL, NULL}
};

const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&rtt_str,
  commands
};
