#include "shell_user.h"
#include "utils.h"
#include "usbcfg.h"
#include "chprintf.h"
#include "stdlib.h"
#include <string.h>
#include "serial_user.h"

thread_t* shelltp = NULL;
static const char* address = "http://www.makahiya.herokuapp.com";
static const char* get = "http_get ";
static const char* post = "http_post ";
static const char* download = "http_download ";

void serial_start(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(chp);
    UNUSED(argc);
    UNUSED(argv);
    sdStart(&SD3, &serial_cfg);
}

void serial_stop(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(chp);
    UNUSED(argc);
    UNUSED(argv);
    sdStop(&SD3);
}

void send(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(chp);
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
    sdWrite(&SD3, serial_tx_buffer, length);
}

void post_led(BaseSequentialStream* chp, int argc, char* argv[]) {
    if (argc != 2)
        chprintf(chp, "Usage: post_led <plant_id> <led_id>/<led_color>/<value>\r\n");
    else {
        int length = 0;
        strcpy((char*)serial_tx_buffer, post);
        length += strlen(post);
        strcat((char*)serial_tx_buffer, address);
        length += strlen(address);
        strcat((char*)serial_tx_buffer, "/api/v1/");
        length += strlen("/api/v1/");
        strcat((char*)serial_tx_buffer, argv[0]);
        length += strlen(argv[0]);
        strcat((char*)serial_tx_buffer, "/actions/led/");
        length += strlen("/actions/led/");
        strcat((char*)serial_tx_buffer, argv[1]);
        length += strlen(argv[1]);
        strcat((char*)serial_tx_buffer, " /application/json\n");
        length += strlen("/application/json\n");
        sdWrite(&SD3, serial_tx_buffer, length);
    }
}

void read_music(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(chp);
    UNUSED(argc);
    UNUSED(argv);
    int length = 0;
    strcpy((char*)serial_tx_buffer, get);
    length += strlen(get);
    strcat((char*)serial_tx_buffer, address);
    length += strlen(address);
    strcat((char*)serial_tx_buffer, "/static/Jens_East_-_Daybreak_feat_Henk.mp3\n");
    length += strlen("/static/Jens_East_-_Daybreak_feat_Henk.mp3\n");
    sdWrite(&SD3, serial_tx_buffer, length);
}

const ShellCommand commands[] = {
  {"serial_start", serial_start},
  {"serial_stop", serial_stop},
  {"send", send},
  {"post_led", post_led},
  {"read_music", read_music},
  {NULL, NULL}
};

const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1,
  commands
};
