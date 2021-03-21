#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>

#include "sclog4c/sclog4c.h"

#include "sensors_control/hammer_angle_sensor.h"

// Hammer Angle Sensor is conned to BBAI P9.33

static const int8_t k_is_hammer_angle_sensor_present = 1;
static const char *k_hammer_angle_sensor_device = "/sys/bus/iio/devices/iio:device0/in_voltage7_raw";

static const int32_t k_min_angle_rad = 0;
static const int32_t k_max_angle_rad = M_PI;

// these voltages should be measured from the actual sensor (via multimeter) and entered here
// TODO: Move these values to toml config file

static const float k_reference_voltage = 3.3f;
static const float k_measured_voltage_at_min_angle = 2.8f;
static const float k_measured_voltage_at_max_angle = 0.9f;

float lerp(float v0, float v1, float t) {
  return (1.0f - t) * v0 + t * v1;
}

int8_t is_hammer_angle_sensor_present()
{
    return k_is_hammer_angle_sensor_present;
}

const char* get_hammer_angle_sensor_device()
{
    return k_hammer_angle_sensor_device;
}

float process_raw_hammer_angle(int32_t sensor_value)
{
    float sensor_value_at_min_angle = (k_measured_voltage_at_min_angle * 4096.0f) / k_reference_voltage;
    float sensor_value_at_max_angle = (k_measured_voltage_at_max_angle * 4096.0f) / k_reference_voltage;

    // map from count to angle
    
    float t = ((float)sensor_value - sensor_value_at_min_angle) /
        (sensor_value_at_max_angle - sensor_value_at_min_angle);

    float hammer_angle = lerp(k_min_angle_rad, k_max_angle_rad, t);


    logm(SL4C_DEBUG, "Hammer Angle = %f", hammer_angle);
    return hammer_angle;
}
