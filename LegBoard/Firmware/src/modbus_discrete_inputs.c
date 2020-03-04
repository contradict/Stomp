#include "modbus.h"
#include "modbus_common.h"

static uint16_t scratchpad = 0x55;

struct MODBUS_DiscreteInput modbus_discrete_inputs[] = {
    {
        .address = 0x55,
        .context = &scratchpad,
        .read = return_context_bool,
    },
    {
        .address = 0,
        .context = 0,
        .read = 0,
    }
};
