#pragma once

#include <stdint.h>
#include <stdbool.h>
void is31fl3235_Init(uint16_t address);
void is31fl3235_Reset();
void is31fl3235_Shutdown(bool shutdown);
void is31fl3235_Update();
void is31fl3235_Enable(bool enable);
void is31fl3235_Set(uint8_t start_channel, uint8_t num_channels, uint8_t *data);
void is31fl3235_SetControl(uint8_t start_channel, uint8_t num_channels, uint8_t *data);
