#pragma once
#include <stdint.h>

struct SensorParameters {
    int32_t velocityFilterCoefficient;
    int16_t maxHammerAngleSpike;
};

extern struct SensorParameters sensor_parameters;

void setSensorParameters(int32_t velocityFilterCoefficient, int16_t maxHammerAngleSpike);
void restoreSensorParameters(void);
