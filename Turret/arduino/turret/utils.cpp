#include "Arduino.h"
#include "utils.h"
#include "pins.h"

void safeDigitalWrite( uint32_t ulPin, uint32_t ulVal){
  if (g_enabled){
    digitalWrite(ulPin, ulVal);
  }
}

int16_t clip(int16_t x, int16_t min, int16_t max) {
    if(x<min) return min;
    if(x>max) return max;
    return x;
}

int32_t clip(int32_t x, int32_t min, int32_t max) {
    if(x<min) return min;
    if(x>max) return max;
    return x;
}
