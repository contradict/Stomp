#include <stdint.h>
#include <stddef.h>

extern uint16_t command_overrun;
extern uint16_t invalid_command;
extern uint16_t valid_command;
extern uint8_t last_command;

void handle_commands(void);
