#pragma once
#include <modbus.h>

int configure_modbus_context(modbus_t *ctx, uint32_t custom_baud, uint32_t response_timeout);
int create_modbus_interface(char *devname, uint32_t custom_baud, uint32_t response_timeout, modbus_t **ctx);
