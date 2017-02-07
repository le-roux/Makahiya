#include "alarm.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include "sound.h"
#include "pwmdriver.h"
#include "RTT_streams.h"
#include "commands.h"
#include "chprintf.h"

/**
 * @brief Virtual timer to set when creating an alarm clock.
 */
static virtual_timer_t alarm_clock;

/**
 * @brief Maximum number of commands that can be executed on alarm_clock expiry.
 */
#define MAX_COMMANDS 128

/**
 * @brief Buffer used by the commands_box mailbox.
 */
static msg_t commands[MAX_COMMANDS];

/**
 * @brief The mailbox storing the commands to execute when alarm_clock expires.
 */
static MAILBOX_DECL(commands_box, commands, MAX_COMMANDS);

/**
 * @brief Callback called when alarm_clock expires.
 */
static void alarm_cb(void* arg);

static BSEMAPHORE_DECL(alarm_bsem, true);

static THD_WORKING_AREA(alarm_wa, 512);
static THD_FUNCTION(alarm, arg);

void alarmInit(void) {
    chVTObjectInit(&alarm_clock);
    chThdCreateStatic(alarm_wa, sizeof(alarm_wa), NORMALPRIO, alarm, NULL);
}

static THD_FUNCTION(alarm, arg) {
    UNUSED(arg);

    // The command to perform. It contains both @p var_id and @p value.
    static msg_t command;

    static loop_t loop;

    while(true) {
        loop.state = SINGLE;
        chBSemWait(&alarm_bsem);

        while(true) {
            if (chMBFetch(&commands_box, &command, MS2ST(5)) != MSG_OK)
                break;

            handle_commands((uint32_t)command, &loop);

            if (loop.state == LOOP)
                chMBPost(&commands_box, command, TIME_INFINITE);
        }
    }
}

void set_alarm(int timeout, char* commands_list, char** save_ptr) {
    /**
     * The id of the variable the command affects.
     */
    uint16_t var_id;

    /**
     * The value that must be set to the variable specified in @p var_id.
     */
    uint16_t value;

    int commands_nb;

    chVTReset(&alarm_clock);
    commands_nb = atoi(commands_list);
    for (int i = 0; i < commands_nb; i++) {
        var_id = (uint16_t)atoi(strtok_r(NULL, " ", save_ptr));
        value = (uint16_t)atoi(strtok_r(NULL, " ", save_ptr));
        commands[i] = ((var_id & 0xFFFF) << 16) | (value & 0xFFFF);
        (void)chMBPost(&commands_box, commands[i], MS2ST(100));
    }
    chVTSet(&alarm_clock, S2ST(timeout), alarm_cb, NULL);
}

static void alarm_cb(void* arg) {
    UNUSED(arg);

    chSysLockFromISR();
    chBSemSignalI(&alarm_bsem);
    chSysUnlockFromISR();
}
