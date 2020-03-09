#include "modbus.h"
#include "modbus_common.h"

static uint16_t scratchpad = 0x55;

static int return_context(void *ctx, bool *v);
static int save_to_context(void *ctx, bool value);

const struct MODBUS_Coil modbus_coils[] = {
    {
        .address = 0x55,
        .context = &scratchpad,
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

static int return_context(void *ctx, bool *v)
{
    *v = *(bool *)ctx;
    return 0;
}

static int save_to_context(void *ctx, bool value)
{
    *(uint16_t *)ctx = value;
    return 0;
}
