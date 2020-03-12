#include "modbus.h"
#include "export/modbus_register_map.h"

#define ENFIELD_CONTEXT_VALUE(j, w, r) ((void *)(((j&3)<<30) | ((w&0xff)<<8) | (r&0xff)))

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
