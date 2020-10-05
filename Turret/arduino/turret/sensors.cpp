#include "sensors.h"
#include <avr/eeprom.h>

struct SensorParameters sensor_parameters;

static struct SensorParameters EEMEM saved_sensor_parameters = 
{
    .velocityFilterCoefficient = 200,
    .maxHammerAngleSpike = 100,
};

void setSensorParameters(int32_t velocityFilterCoefficient, int16_t maxHammerAngleSpike)
{
    sensor_parameters.velocityFilterCoefficient = velocityFilterCoefficient;
    sensor_parameters.maxHammerAngleSpike = maxHammerAngleSpike;
    eeprom_write_block(&sensor_parameters, &saved_sensor_parameters, sizeof(struct SensorParameters));
}

void restoreSensorParameters(void)
{
    eeprom_read_block(&sensor_parameters, &saved_sensor_parameters, sizeof(struct SensorParameters));
}
