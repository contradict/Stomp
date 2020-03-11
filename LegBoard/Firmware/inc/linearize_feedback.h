#pragma once
#include <stdint.h>
#include "modbus.h"

void Linearize_ThreadInit(void);
int Linearize_ReadAngle(void *ctx, uint16_t *v);
int Linearize_ReadLength(void *ctx, uint16_t *v);
