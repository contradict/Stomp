#include "modbus_common.h"

int return_context(void *context, uint16_t *v)
{
    *v = *(uint16_t *)context;
    return 0;
}

int return_context_bool(void *context, bool *v)
{
    *v = *(bool *)context;
    return 0;
}

int save_to_context(void *context, uint16_t value)
{
    *(uint16_t *)context = value;
    return 0;
}

int save_to_context_bool(void *context, bool v)
{
    *(bool *)context = v;
    return 0;
}
