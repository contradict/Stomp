#pragma once

#include <stdint.h>
#include <stdbool.h>

#define IS31FL3235_NUM_CHANNELS 28

#define IS31FL3235_ADDR_GND 0x78
#define IS31FL3235_ADDR_VCC 0x7E
#define IS31FL3235_ADDR_SCL 0x7A
#define IS31FL3235_ADDR_SDA 0x7C


void is31fl3235_Init(uint16_t address);
void is31fl3235_Reset();
void is31fl3235_Shutdown(bool shutdown);
void is31fl3235_Update();
void is31fl3235_Enable(bool enable);
void is31fl3235_Set(uint8_t start_channel, uint8_t num_channels, uint8_t *data);
void is31fl3235_SetControl(uint8_t start_channel, uint8_t num_channels, uint8_t *data);
