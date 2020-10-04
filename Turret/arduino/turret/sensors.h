#pragma once
#include <stdint.h>

struct SensorParameters {
    int32_t velocityFilterCoefficient;
};

extern struct SensorParameters sensor_parameters;

void setSensorParameters(int32_t velocityFilterCoefficient);
void restoreSensorParameters(void);
