#include "ch.h"
#include "hal.h"

#include "websocket.h"
#include "utils.h"
#include "wifi.h"

#include "chprintf.h"
#include "usbcfg.h"
#include <string.h>

static const char* const ws_cmd = "websocket_client -g 14 ";
static const char* const ws_addr = "ws://makahiya.rfc1149.net:9000/ws/plants/";
static const char* const poll_cmd = "poll ";
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
    // Start the EXT driver (for interrupt)
    extStart(&EXTD1, &ext_cfg);
    DEBUG("Starting callback thread");
    while(TRUE) {
        DEBUG("enter while");
        chBSemWait(&web_bsem);
        read_buffer((wifi_connection)conn);
        DEBUG("read");
        header = get_response(true);
        if (header.error)
            DEBUG("cb error %s", response_code);
        else
            DEBUG("%s", response_body);
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
            DEBUG("error: %i", header.error_code);
            chThdSleepMilliseconds(2000);
        } else
            break; // Connection established
    }

    get_channel_id((wifi_connection*)&conn);
    chThdCreateStatic(wa_websocket_ext, sizeof(wa_websocket_ext), \
    NORMALPRIO, websocket_ext, NULL);

    while (true) {
        chThdSleepMilliseconds(3000);
        wifi_write((wifi_connection*)&conn, 5, "abcd");
        get_response(true);
        DEBUG("write %s", response_body);
    }
}
