#include "ch.h"
#include "hal.h"

#include "websocket.h"
#include "utils.h"
#include "wifi.h"

#include "serial_user.h"
#include <stdlib.h>
#include <string.h>
#include "alarm.h"
#include "sound.h"
#include "ext_user.h"
#include "RTT_streams.h"
#include "chprintf.h"

// 14 is the number of the pin used for the interrupts.
static const char* const ws_cmd = "websocket_client -g 9 ";
static const char* const ws_addr = "ws://makahiya.rfc1149.net:9000/ws/plants/";

static volatile wifi_connection conn;

static BSEMAPHORE_DECL(web_bsem, true);
void set_value(int var_id, int value);
int get_value(int var_id);

void web_cb(EXTDriver* driver, expchannel_t channel) {
    UNUSED(driver);
    UNUSED(channel);
    if (palReadPad(GPIOA, GPIOA_WIFI_RTS) == PAL_LOW)
        return; // It was just noise.
    chSysLockFromISR();
    chBSemSignalI(&web_bsem);
    chSysUnlockFromISR();
}

static THD_WORKING_AREA(wa_websocket_ext, WA_WEBSOCKET_SIZE);
THD_FUNCTION(websocket_ext, arg) {
    UNUSED(arg);
    wifi_response_header header;
    char *cmd, *var;
    char value[5], buffer[20];
    // Start the EXT driver (for interrupt)
    extStart(EXTD, &ext_config);
    while(TRUE) {
        chBSemWait(&web_bsem); // Wait for a trigger from the interrupt
        read_buffer(conn); // Ask to read the data
        header = get_response(true);
        if (header.error) {
            continue;
        }

        // No error: decode the data received
        cmd = strtok(response_body, " ");
        if (cmd == NULL)
            continue; // TODO improve error management
        if (strcmp(cmd, "get") == 0 || strcmp(cmd, "set") == 0) {
            var = strtok(NULL, " ");
            if (strcmp(cmd, "set") == 0)
                set_value(atoi(var), atoi(strtok(NULL, " ")));

            // Same actions for 'get' and 'set'
            int_to_char(value, get_value(atoi(var)));
            strcpy(buffer, var);
            strcat(buffer, " ");
            strcat(buffer, value);
            wifi_write((wifi_connection*)&conn, strlen(buffer), (uint8_t*)buffer);
            (void)get_response(true);
        } else if (strcmp(cmd, "play") == 0) {
            // TODO Play music file called $var
            continue;
        } else if (strcmp(cmd, "alarm") == 0) {
            int timeout;
            var = strtok(NULL, " ");
            if (var == NULL)
                continue;
            timeout = atoi(var);
            set_alarm(timeout, strtok(NULL, " "));
        } else if (strcmp(cmd, "stop") == 0) {
        }
    }
    repeat = 0;
}

/**
 * Main websocket thread.
 */
THD_WORKING_AREA(wa_websocket, WA_WEBSOCKET_SIZE);

THD_FUNCTION(websocket, arg) {

    char cmd[100];
    wifi_response_header header;

    chThdSleepMilliseconds(1000);
    strcpy(cmd, ws_cmd);
    strcat(cmd, ws_addr);
    strcat(cmd, (char*)arg);

    while(true) {
        // Open the connection with the websocket (plant-side).
        send_cmd(cmd);
        /* No timeout because this first connection can be long to establish.
           However it's not fully blocking as the wifi module will respond
           with a "command failed" message after a certain amount of time in
           case of error. */
        header = get_response(false);
        if (header.error) {
            DEBUG("error %s (%s)", response_body, response_code);
            do {
                if (header.error_code == SAFEMODE) {
                    exit_safe_mode();
                    header.error = NO_ERROR;
                    header.error_code = NO_ERROR;
                } else {
                    send_cmd(REBOOT);
                    header = get_response(false);
                    if (header.error)
                        continue;
                    send_cmd(NETWORK_FLUSH);
                    header = get_response(false);
                    if (header.error)
                        continue;
                }
            } while (header.error);
            do {
                send_cmd(PING_CONN);
                header = get_response(false);
            } while(header.error);
        } else
            break; // Connection established
    }

    get_channel_id((wifi_connection*)&conn);
    chThdCreateStatic(wa_websocket_ext, sizeof(wa_websocket_ext), \
                        NORMALPRIO, websocket_ext, NULL);

    // For test only
    for (int i = 0; i < 5; i++) {
        chThdSleepMilliseconds(5000);
        wifi_write((wifi_connection*)&conn, 4, (uint8_t*)"abcd");
        (void)get_response(true);
        DEBUG("%s", response_body);
    }
    strcpy(cmd, "close ");
    strcat(cmd, ((wifi_connection)conn).channel_id);
    send_cmd(cmd);
    header = get_response(false);
    DEBUG("close %s", response_body);
}

void set_value(int var_id, int value) {
     UNUSED(var_id);
     UNUSED(value);
}
int get_value(int var_id) {
    UNUSED(var_id);
    return 1;
}