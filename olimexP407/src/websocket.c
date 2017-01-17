#include "ch.h"
#include "hal.h"

#include "websocket.h"
#include "utils.h"
#include "wifi.h"

#include "chprintf.h"
#include "usbcfg.h"
#include "logic.h"
#include "serial_user.h"
#include <stdlib.h>
#include <string.h>
#include "alarm.h"

static const char* const ws_cmd = "websocket_client -g 14 ";
static const char* const ws_addr = "ws://makahiya.rfc1149.net:9000/ws/plants/";
static char cmd[100];

static volatile wifi_connection conn;

static BSEMAPHORE_DECL(web_bsem, true);
void ext_cb(EXTDriver* driver, expchannel_t channel);

static const EXTConfig ext_cfg = {
    {
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOE, ext_cb},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL}
    }
};

void ext_cb(EXTDriver* driver, expchannel_t channel) {
    UNUSED(driver);
    UNUSED(channel);
    if (palReadPad(GPIOE, 7) == PAL_LOW)
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
    extStart(&EXTD1, &ext_cfg);
    DEBUG("Starting callback thread");
    while(TRUE) {
        chBSemWait(&web_bsem); // Wait for a trigger from the interrupt
        read_buffer((wifi_connection)conn); // Ask to read the data
        header = get_response(true);
        if (header.error) {
            DEBUG("cb error %s", response_code);
            continue;
        }

        // No error: decode the data received
        cmd = strtok(response_body, " ");
        if (cmd == NULL)
            continue; // TODO improve error management
        if (strcmp(cmd, "get") == 0 || strcmp(cmd, "set") == 0) {
            var = strtok(NULL, " ");
            if (strcmp(cmd, "set") == 0)
                set_value(var, atoi(strtok(NULL, " ")));

            // Same actions for 'get' and 'set'
            int_to_char(value, get_value(var));
            strcpy(buffer, var);
            strcat(buffer, " ");
            strcat(buffer, value);
            DEBUG("response -> %s (%i)", buffer, strlen(buffer));
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
        }
    }
}

/**
 * Main websocket thread.
 */
THD_WORKING_AREA(wa_websocket, WA_WEBSOCKET_SIZE);

THD_FUNCTION(websocket, arg) {

    wifi_response_header header;

    chThdSleepMilliseconds(2000);
    strcpy(cmd, ws_cmd);
    strcat(cmd, ws_addr);
    strcat(cmd, (char*)arg);



    while(true) {
        // Open the connection with the websocket (plant-side).
        send_cmd(cmd);
        header = get_response(false);
        if (header.error) {
            do {
                send_cmd("reboot");
                header = get_response(false);
                chThdSleepMilliseconds(750);
            } while (header.error);
            do {
                send_cmd("ping -g");
                header = get_response(false);
                chThdSleepMilliseconds(250);
            } while(header.error);
        } else
            break; // Connection established
    }

    get_channel_id((wifi_connection*)&conn);
    chThdCreateStatic(wa_websocket_ext, sizeof(wa_websocket_ext), \
    NORMALPRIO, websocket_ext, NULL);

    while (true) {
        chThdSleepMilliseconds(5000);
        wifi_write((wifi_connection*)&conn, 4, (uint8_t*)"abcd");
        (void)get_response(true);
        DEBUG("write %s", response_body);
    }
}
