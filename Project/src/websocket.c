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
#include "pwmdriver.h"
#include "fdc2214.h"

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
        header = read_buffer(conn, true); // Ask to read the data
        if (header.error) {
            continue;
        }

        // No error: decode the data received
        DEBUG("Received %s", response_body);
        char* save_ptr;
        cmd = strtok_r(response_body, " ", &save_ptr);
        if (cmd == NULL)
            continue;
        do {
            if (strcmp(cmd, "set") == 0) {
                var = strtok_r(NULL, " ", &save_ptr);
                setValue(atoi(var), atoi(strtok_r(NULL, " ", &save_ptr)));
                DEBUG("set");
            } else if (strcmp(cmd, "get") == 0) {
                var = strtok_r(NULL, " ", &save_ptr);
                int_to_char(value, getValue(atoi(var)));
                strcpy(buffer, var);
                strcat(buffer, " ");
                strcat(buffer, value);
                (void)wifi_write((wifi_connection*)&conn, strlen(buffer), (uint8_t*)buffer, true);
            } else if (strcmp(cmd, "play") == 0) {
                var = strtok_r(NULL, " ", &save_ptr);
                urgent_stop = false;
                repeat = 1;
                read_music(var);
            } else if (strcmp(cmd, "alarm") == 0) {
                int timeout;
                var = strtok_r(NULL, " ", &save_ptr);
                if (var == NULL)
                    continue;
                timeout = atoi(var);
                set_alarm(timeout, strtok_r(NULL, " ", &save_ptr), &save_ptr);
            } else if (strcmp(cmd, "stop") == 0) {
                repeat = 0;
                urgent_stop = true;
                char close_cmd[11];
                strcpy(close_cmd, "close ");
                strcat(close_cmd, ((wifi_connection)audio_conn).channel_id);
                send_cmd(close_cmd, false);
            } else if (strcmp(cmd, "add") == 0) {
                int sensor_id = atoi(strtok_r(NULL, " ", &save_ptr));
                int channel_id = atoi(strtok_r(NULL, " ", &save_ptr));
                int commands_nb = atoi(strtok_r(NULL, " ", &save_ptr));
                int var_id, value;
                for (int i = 0; i < commands_nb; i++) {
                    var_id = atoi(strtok_r(NULL, " ", &save_ptr));
                    value = atoi(strtok_r(NULL, " ", &save_ptr));

                    add_command(sensor_id, channel_id, var_id, value);
                }
            } else if (strcmp(cmd, "clear")) {
                int sensor_id = atoi(strtok_r(NULL, " ", &save_ptr));
                int channel_id = atoi(strtok_r(NULL, " ", &save_ptr));

                clear_commands(sensor_id, channel_id);
            }
            cmd = strtok_r(NULL, " ", &save_ptr);
        } while (cmd != NULL);
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
        /* Open the connection with the websocket (plant-side).
         No timeout because this first connection can be long to establish.
        However it's not fully blocking as the wifi module will respond
        with a "command failed" message after a certain amount of time in
        case of error. */
        header = send_cmd(cmd, false);
        if (header.error) {
            DEBUG("error %s (%s)", response_body, response_code);
            do {
                if (header.error_code == SAFEMODE) {
                    exit_safe_mode();
                    header.error = NO_ERROR;
                    header.error_code = NO_ERROR;
                } else {
                    header = send_cmd(REBOOT, false);
                    if (header.error)
                        continue;
                    header = send_cmd(NETWORK_FLUSH, false);
                    if (header.error)
                        continue;
                }
            } while (header.error);
            do {
                header = send_cmd(PING_CONN, false);
            } while(header.error);
        } else
            break; // Connection established
    }

    get_channel_id((wifi_connection*)&conn);
    chThdCreateStatic(wa_websocket_ext, sizeof(wa_websocket_ext), \
                        NORMALPRIO, websocket_ext, NULL);

    header = wifi_write((wifi_connection*)&conn, 4, (uint8_t*)"sync", false);
    DEBUG("sync %s", response_body);

    while (true) {
        chThdSleepMilliseconds(10000);
        header = wifi_write((wifi_connection*)&conn, 4, (uint8_t*)"abcd", true);
        DEBUG("%s", response_body);
        if (header.error && header.error_code == COMMAND_FAILED)
            break;
    }
}

void websocket_write(char* buffer) {
	wifi_write((wifi_connection*)&conn, strlen(buffer), (uint8_t*)buffer, false);
}
