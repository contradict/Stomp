#include <modbus.h>
#include "cmsis_os.h"

#include "modbus_common.h"

static uint16_t scratchpad = 0x55;

static const uint8_t EnfieldHoldingRegister[] = {
};

static int return_context(void *, uint16_t *v);
static int save_to_context(void *, uint16_t value);

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

static int return_context(void *ctx, uint16_t *v)
{
    *v = *(uint16_t *)ctx;
    return 0;
}

static int save_to_context(void *ctx, uint16_t value)
{
    *(uint16_t *)ctx = value;
    return 0;
}

