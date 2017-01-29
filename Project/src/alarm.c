#include "alarm.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include "sound.h"

/**
 * @brief Virtual timer to set when creating an alarm clock.
 */
static virtual_timer_t alarm_clock;

/**
 * @brief Maximum number of commands that can be executed on alarm_clock expiration.
 */
#define MAX_COMMANDS 16

/**
 * @brief Buffer used by the commands_box mailbox.
 */
static msg_t commands[MAX_COMMANDS];

/**
 * @brief The mailbox storing the commands to execute when alarm_clock expires.
 */
static MAILBOX_DECL(commands_box, commands, MAX_COMMANDS);

/**
 * @brief The number of commands to execute when alarm_clock expires.
 */
static int commands_nb;

/**
 * @brief Callback called when alarm_clock expires.
 */
static void alarm_cb(void* arg);

#define MUSIC 1

void alarm_init(void) {
    chVTObjectInit(&alarm_clock);
}

void set_alarm(int timeout, char* commands_list) {
    /**
     * The id of the variable the command affects.
     */
    uint16_t var_id;

    /**
     * The value that must be set to the variable specified in @p var_id.
     */
    uint16_t value;

    chVTReset(&alarm_clock);
    commands_nb = atoi(commands_list);
    for (int i = 0; i < commands_nb; i++) {
        var_id = (uint16_t)atoi(strtok(NULL, " "));
        value = (uint16_t)atoi(strtok(NULL, " "));
        commands[i] = ((var_id & 0xFFFF) << 16) | (value & 0xFFFF);
        (void)chMBPost(&commands_box, commands[i], TIME_INFINITE);
    }
    chVTSet(&alarm_clock, S2ST(timeout), alarm_cb, NULL);
}

static void alarm_cb(void* arg) {
    UNUSED(arg);

    /**
     * The command to perform. It contains both @p var_id and @p value.
     */
    msg_t command;

    /**
     * The id of the variable the command affects.
     */
    uint16_t var_id;

    /**
     * The value that must be set to the variable specified in @p var_id.
     */
    uint16_t value;

    for (int i = 0; i < commands_nb; i++) {
        chSysLockFromISR();
        (void)chMBFetchI(&commands_box, &command);
        chSysUnlockFromISR();
        var_id = (uint16_t)((command & 0xFFFF0000) >> 16);
        value = (uint16_t)(command & 0xFFFF);
        switch (var_id) {
            case(MUSIC): {
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
