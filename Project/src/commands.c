#include "commands.h"

#include "hal.h"
#include "sound.h"
#include "pwmdriver.h"
#include "websocket.h"

#define RINGBELL_CMD 1
#define SLEEP_CMD 250
#define LOOP_CMD 251
static const int STOP_MUSIC = 255;

void handle_commands(uint32_t command, loop_t* loop) {
    // The id of the variable the command affects.
    uint16_t var_id;

    // The value that must be set to the variable specified in @p var_id.
    uint16_t value;

    var_id = (uint16_t)((command & 0xFFFF0000) >> 16);
    value = (uint16_t)(command & 0xFFFF);
    switch (var_id) {
        case RINGBELL_CMD: {
            if (value == STOP_MUSIC) {
                repeat = false;
            } else if (value == ALARM_SOUND_NB) {
                websocket_write("music"); 
            } else {
                music_id = value;
                repeat = true;
                chBSemSignal(&audio_bsem);
            }
            break;
        }
        case LOOP_CMD: {
            if (loop->state == SINGLE) {
                loop->count = value - 1;
                loop->first_turn = true;
                loop->state = LOOP;
            } else {
                loop->count--;
                loop->first_turn = false;
                if (loop->count == 0)
                    loop->state = SINGLE;
            }
            break;
        }
        case SLEEP_CMD: {
            chThdSleepMilliseconds(value);
            break;
        }
        default: {
            setValue(var_id, value);
        }
    }
}
