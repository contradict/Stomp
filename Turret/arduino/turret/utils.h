#ifndef UTILS_H
#define UTILS_H

void safeDigitalWrite( uint32_t ulPin, uint32_t ulVal); // matches Arduino

int16_t clip(int16_t x, int16_t min, int16_t max);
int32_t clip(int32_t x, int32_t min, int32_t max);

#endif
