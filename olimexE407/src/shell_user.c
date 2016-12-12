#include "shell_user.h"
#include "utils.h"
#include "usbcfg.h"
#include "chprintf.h"
#include "stdlib.h"
#include <string.h>
#include "serial_user.h"
#include "wifi.h"

thread_t* shelltp = NULL;
static const char* address = "http://www.makahiya.herokuapp.com";
static const char* get = "http_get ";
static const char* post = "http_post ";
static const char* download = "http_download ";
static const char* read = "read ";

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

    // Actually send the request
    sdWrite(&SD3, serial_tx_buffer, length);

    // Read the response code
    sdRead(&SD3, (uint8_t*)response_code, WIFI_HEADER_SIZE);
    wifi_response out = parse_response_code();
    if (out.error == 1)
        chprintf(chp, "Error (code: %i)\r\n", out.error_code);
    else
        chprintf(chp, "Success (size: %i)\r\n", out.length);
    if (out.error_code != -1) {
        // Read the channel id given to this connection.
        sdRead(&SD3, (uint8_t*)response_body, out.length);
        response_body[out.length] = '\0';
        chprintf(chp, "Body: %s", response_body);
        get_channel_id(response_body, out.channel_id);
    }

    /**
     * Read the content of the mp3 file.
     */
    length = 0;

    char tmp[3];
    itoa(WIFI_BUFFER_SIZE - 2, tmp, 10);

    // Prepare the read request.
    strcpy((char*)serial_tx_buffer, read);
    length += strlen(read);
    strcat((char*)serial_tx_buffer, out.channel_id);
    length += strlen(out.channel_id);
    strcat((char*)serial_tx_buffer, tmp);
    length += strlen(tmp);

    do {
        // Actually send the read request
        sdWrite(&SD3, serial_tx_buffer, length);

        // Read the response
        sdRead(&SD3, (uint8_t*)response_code, WIFI_HEADER_SIZE);
        out = parse_response_code();
        if (out.error)
          return; // TODO improve error management
        sdRead(&SD3, (uint8_t*)response_body, out.length);
    } while (out.length == WIFI_BUFFER_SIZE);
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
