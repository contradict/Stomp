#include <stdint.h>

void LED_ThreadInit(void);
void LED(uint8_t idx, uint8_t r, uint8_t g, uint8_t b);
void LED_TestPatternStart(void);
void LED_TestPatternStop(void);
