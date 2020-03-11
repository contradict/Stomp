#include "modbus.h"
#include "modbus_common.h"

static uint16_t scratchpad = 0x55;

static int return_context(void *ctx, bool *v);

const struct MODBUS_DiscreteInput modbus_discrete_inputs[] = {
    {
        .address = 0x55,
        .context = &scratchpad,
        .read = return_context,
    },
    {
        .address = 0,
        .context = 0,
        .read = 0,
    }
};

static int return_context(void *ctx, bool *v)
{
    *v = *(bool *)ctx;
    return 0;
}
