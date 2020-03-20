#pragma once
#include <modbus.h>

int configure_modbus_context(modbus_t *ctx, uint32_t custom_baud, uint32_t response_timeout);
