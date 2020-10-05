#include "sensors.h"
#include <avr/eeprom.h>

struct SensorParameters sensor_parameters;

static struct SensorParameters EEMEM saved_sensor_parameters = 
{
    .velocityFilterCoefficient = 100,
};

void setSensorParameters(int32_t velocityFilterCoefficient)
{
    sensor_parameters.velocityFilterCoefficient = velocityFilterCoefficient;
    eeprom_write_block(&sensor_parameters, &saved_sensor_parameters, sizeof(struct SensorParameters));
}

void restoreSensorParameters(void)
{
    eeprom_read_block(&sensor_parameters, &saved_sensor_parameters, sizeof(struct SensorParameters));
}
