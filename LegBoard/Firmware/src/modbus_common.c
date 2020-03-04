#include "modbus_common.h"

uint16_t return_context(void *context)
{
    return *(uint16_t *)context;
}

bool return_context_bool(void *context)
{
    return *(bool *)context;
}

void save_to_context(void *context, uint16_t value)
{
    *(uint16_t *)context = value;
}

void save_to_context_bool(void *context, bool v)
{
    *(bool *)context = v;
}
