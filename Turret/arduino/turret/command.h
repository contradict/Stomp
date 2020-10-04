#pragma once

#include <stdint.h>
#include <stddef.h>

enum Commands {
    CMD_UNKNOWN = 0,
    CMD_ID_TRATE = 10,
    CMD_ID_TRKFLT = 11,
    CMD_ID_OBJSEG = 12,
    CMD_ID_AF = 13,
    CMD_ID_AAIM = 14,
    CMD_ID_IMUP = 15,
    CMD_ID_LDDR = 17,
    CMD_ID_HMR = 18,
    CMD_ID_TRT = 19,
    CMD_ID_TROT = 20,
    CMD_ID_SNSP = 21
};


extern uint16_t command_overrun;
extern uint16_t invalid_command;
extern uint16_t valid_command;
extern enum Commands last_command;

void handleCommands(void);
