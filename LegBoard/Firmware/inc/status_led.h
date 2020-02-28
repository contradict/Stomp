#include <stdint.h>

void LED_ThreadInit(void);
void LED(uint8_t idx, uint8_t r, uint8_t g, uint8_t b);
void LED_TestPatternStart(void);
void LED_TestPatternStop(void);
void LED_R(uint8_t idx, uint8_t r);
void LED_G(uint8_t idx, uint8_t g);
void LED_B(uint8_t idx, uint8_t b);
void LED_Raw(uint8_t idx, uint8_t v);
