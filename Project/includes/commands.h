#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {SINGLE, LOOP} commands_state_t;

typedef struct {
    int count;
    bool first_turn;
    commands_state_t state;
} loop_t;

void handle_commands(uint32_t command, loop_t* loop);

#endif // COMMANDS_H
