#include <modbus.h>

static uint16_t scratchpad = 0x55;

uint16_t return_context(void *context)
{
    return *(uint16_t *)context;
}

void save_to_context(void *context, uint16_t value)
{
    *(uint16_t *)context = value;
}

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
