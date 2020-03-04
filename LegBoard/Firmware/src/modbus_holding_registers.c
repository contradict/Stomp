#include <modbus.h>

#include "modbus_common.h"

static uint16_t scratchpad = 0x55;

struct MODBUS_HoldingRegister modbus_holding_registers[] = {
    {
        .address = 0x55,
        .context = (void *)&scratchpad,
        .read = return_context,
        .write = save_to_context
    },
    {
        .address = 0,
        .context = 0,
        .read = 0,
        .write = 0
    }
};
