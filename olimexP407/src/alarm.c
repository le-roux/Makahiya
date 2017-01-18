#include "alarm.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include "usbcfg.h"
#include "chprintf.h"
#include "sound.h"

/**
 * @brief Virtual timer to set when creating an alarm clock.
 */
static virtual_timer_t alarm_clock;

/**
 * @brief Buffer used by the commands_box mailbox.
 */
static msg_t commands[MAX_COMMANDS];

/**
 * @brief The mailbox storing the commands to execute when alarm_clock expires.
 */
static MAILBOX_DECL(commands_box, commands, MAX_COMMANDS);
MUTEX_DECL(lock);

static int commands_nb;
int alarm_activated;

static void alarm_cb(void* arg);

void alarm_init(void) {
    chMtxObjectInit(&lock);
    alarm_activated = false;
    chVTObjectInit(&alarm_clock);
}

void set_alarm(int timeout, char* commands_list) {
    uint16_t var_id, value;
    chVTReset(&alarm_clock);
    commands_nb = atoi(commands_list);
    for (int i = 0; i < commands_nb; i++) {
        var_id = (uint16_t)atoi(strtok(NULL, " "));
        value = (uint16_t)atoi(strtok(NULL, " "));
        commands[i] = ((var_id & 0xFFFF) << 16) | (value & 0xFFFF);
        (void)chMBPost(&commands_box, commands[i], TIME_INFINITE);
    }
    chMtxLock(&lock);
    alarm_activated = true;
    chMtxUnlock(&lock);
    DEBUG("timeout %i", timeout);
    chVTSet(&alarm_clock, S2ST(timeout), alarm_cb, NULL);
}

static void alarm_cb(void* arg) {
    UNUSED(arg);
    msg_t command;
    uint16_t var_id, value;
    alarm_activated = false;
    for (int i = 0; i < commands_nb; i++) {
        chSysLockFromISR();
        (void)chMBFetchI(&commands_box, &command);
        chSysUnlockFromISR();
        var_id = (uint16_t)((command & 0xFFFF0000) >> 16);
        value = (uint16_t)(command & 0xFFFF);
        switch (var_id) {
            case(1): {
                music_id = value;
                repeat = 1;
                chSysLockFromISR();
                chBSemSignalI(&audio_bsem);
                chSysUnlockFromISR();
            }
            default: {
                // Do something
                (void)var_id;
            }
        }
    }
}
