#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>

#include "sclog4c/sclog4c.h"

#include "sensors_control/throw_pressure_sensor.h"

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

#define k_max_device_len 512

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static int8_t s_is_turret_angle_sensor_present;
static char s_turret_angle_sensor_device[k_max_device_len];

static float s_min_angle_rad;
static int s_min_angle_count;
static float s_max_angle_rad;
static int s_max_angle_count;

static float s_turret_angle = 0.0f;
static float s_turret_velocity = 0.0f;

// -----------------------------------------------------------------------------
// public methods
// -----------------------------------------------------------------------------

int32_t turret_angle_sensor_init(int8_t present, char* device, int32_t min_angle_deg, float min_angle_volts, int32_t max_angle_deg, float max_angle_volts, float dev_scale, float volt_scale)
{
    s_is_turret_angle_sensor_present = present;

    logm(SL4C_DEBUG, "Initialize Turret Angle Sensor: Is Present %d", s_is_turret_angle_sensor_present);

    if (strlen(device) >= k_max_device_len)
    {
        logm(SL4C_FATAL, "Device String too long");
        return -1;
    }

    strcpy (s_turret_angle_sensor_device, device);

    // Conver degrees to radian

    s_min_angle_rad = min_angle_deg * M_PI/180.0f;
    s_max_angle_rad = max_angle_deg * M_PI/180.0f;
    
    // convert from the actual sensor voltage, down to what the cape analog opamp outputs (volt_scale)
    // which is in Amps.  Then convert to milliamps (A * 1000, because that is what the dev_scale
    // expects) then from voltage to count, scale provided by hardware

    s_min_angle_count = (int32_t)((min_angle_volts * volt_scale * 1000.0f) / dev_scale);
    s_max_angle_count = (int32_t)((max_angle_volts * volt_scale * 1000.0f) / dev_scale);


    return 0;
}
int8_t is_turret_angle_sensor_present()
{
    return s_is_turret_angle_sensor_present;
}

const char* get_turret_angle_sensor_device()
{
    return s_turret_angle_sensor_device;
}

float get_turret_angle()
{
    return s_turret_angle;
}

float get_turret_velocity()
{
    return s_turret_velocity;
}

void calculate_turret_values(int32_t raw_sensor_value)
{
    s_turret_angle = 0.0f;
    s_turret_velocity = 0.0f;
}