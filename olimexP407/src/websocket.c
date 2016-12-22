#include "ch.h"
#include "hal.h"

#include "websocket.h"
#include "utils.h"
#include "wifi.h"

#include "chprintf.h"
#include "usbcfg.h"
#include <string.h>

static const char* const ws_cmd = "websocket_client ";
static const char* const ws_addr = "ws://makahiya.rfc1149.net:9000/ws/plants/";
static const char* const poll_cmd = "poll ";
static char cmd[100];

THD_WORKING_AREA(wa_websocket, WA_WEBSOCKET_SIZE);

THD_FUNCTION(websocket, arg) {

    wifi_response_header header;
    wifi_connection conn;

    chThdSleepMilliseconds(2000);
    while(true) {
        // Open the connection with the websocket (plant-side).
        strcpy(cmd, ws_cmd);
        strcat(cmd, ws_addr);
        strcat(cmd, (char*)arg);
        send_cmd(cmd);
        header = get_response(false);
        if (header.error) {
            DEBUG("error: %i", header.error_code);
            chThdSleepMilliseconds(2000);
            continue;
        }
        get_channel_id(&conn);

        while (true) {
            chThdSleepMilliseconds(1000);
            strcpy(cmd, poll_cmd);
            strcat(cmd, conn.channel_id);
            send_cmd(cmd);
            header = get_response(true);
            if (header.error) {
                DEBUG("error %i", header.error_code);
                continue; //TODO improve error management
            } else {
                DEBUG("res -> %c", response_body[0]);
                if (response_body[0] == DATA_AVAILABLE) {
                    read_buffer(conn);
                    header = get_response(true);
                    if (header.error)
                        continue;
                    else
                        DEBUG("%s", response_body);
                } else if (response_body[0] == CONN_CLOSED) {
                    break;
                }
            }
            strcpy(cmd, "write ");
            strcat(cmd, conn.channel_id);
            strcat(cmd, " 5");
            send_cmd(cmd);
            send_cmd("abcd");
            get_response(true);
            DEBUG("write %s", response_body);
        }
    }
}
