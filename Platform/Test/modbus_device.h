#pragma once
#include <modbus.h>

int configure_modbus_context(modbus_t *ctx, int custom_baud, int32_t response_timeout);
