#include <modbus.h>

#include "modbus_common.h"

static uint16_t scratchpad = 0x55;

struct MODBUS_InputRegister modbus_input_registers[] = {
    {
        .address = 0x55,
        .context = (void *)&scratchpad,
        .read = return_context
    },
    {
        .address = 0,
        .context = 0,
        .read = 0,
    }
};
