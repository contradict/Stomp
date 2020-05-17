#pragma once

#include <stdint.h>

void LED_ThreadInit(void);
void LED_SetRGB(uint8_t idx, uint8_t r, uint8_t g, uint8_t b);
void LED_SetOne(uint8_t idx, uint8_t channel, uint8_t r);
void LED_BlinkRGB(uint8_t idx, uint8_t r, uint8_t g, uint8_t b, uint8_t duration);
void LED_BlinkOne(uint8_t idx, uint8_t channel, uint8_t r, uint8_t duration);
void LED_TestPatternStart(void);
void LED_TestPatternStop(void);
