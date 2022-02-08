#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>

#include "sclog4c/sclog4c.h"
#include "sensors_control/hammer_angle_sensor.h"

// Hammer Angle Sensor is connected to BBAI P9.39 (Chomp BeagleCape A0)

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

#define k_max_device_len 512

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static int8_t s_is_hammer_angle_sensor_present;
static char s_hammer_angle_sensor_device[k_max_device_len];

static float s_min_angle_rad;
static int s_min_angle_count;
static float s_max_angle_rad;
static int s_max_angle_count;

static float s_hammer_angle = 0.0f;
static float s_hammer_angle_prev = 0.0f;
static float s_hammer_velocity = 0.0f;

// -----------------------------------------------------------------------------
//  forward decl of internal methods
// -----------------------------------------------------------------------------

void calculate_hammer_angle(int32_t raw_sensor_value);
void calculate_hammer_velocity();

// -----------------------------------------------------------------------------
// public methods
// -----------------------------------------------------------------------------

int32_t hammer_angle_sensor_init(int8_t present, char* device, int32_t min_angle_deg, float min_angle_volts, int32_t max_angle_deg, float max_angle_volts, float dev_scale, float volt_scale)
{

    s_is_hammer_angle_sensor_present = present;

    logm(SL4C_DEBUG, "Initialize Hammer Angle Sensor: Is Present %d", s_is_hammer_angle_sensor_present);

    if (strlen(device) >= k_max_device_len)
    {
        logm(SL4C_FATAL, "Device String too long");
        return -1;
    }

    strcpy (s_hammer_angle_sensor_device, device);

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

int8_t is_hammer_angle_sensor_present()
{
    return s_is_hammer_angle_sensor_present;
}

const char* get_hammer_angle_sensor_device()
{
    return s_hammer_angle_sensor_device;
}

float get_hammer_angle()
{
    return s_hammer_angle;
}

float get_hammer_velocity()
{
    return s_hammer_velocity;
}

float get_hammer_energy()
{
    return 0.0f;
}

float get_available_break_energy()
{
    return 10.0f;
}

void calculate_hammer_values(int32_t raw_sensor_value)
{
    calculate_hammer_angle(raw_sensor_value);
    calculate_hammer_velocity();
}

// -----------------------------------------------------------------------------
// private methods
// -----------------------------------------------------------------------------

float lerp(float v0, float v1, float t) 
{
  return (1.0f - t) * v0 + t * v1;
}

void calculate_hammer_angle(int32_t raw_sensor_value)
{
    logm(SL4C_DEBUG, "Raw Angle Sensor Value = %d", raw_sensor_value);

    s_hammer_angle_prev = s_hammer_angle;
    
    // map from count to angle
    
    float t = ((float)raw_sensor_value - s_min_angle_count) /
        (s_max_angle_count - s_min_angle_count);

    s_hammer_angle = lerp(s_min_angle_rad, s_max_angle_rad, t);

    logm(SL4C_DEBUG, "rad angle = %f, Hammer Angle = %f deg, t = %f", s_hammer_angle, s_hammer_angle * (180.0f / M_PI), t);
}

void calculate_hammer_velocity()
{
    struct timeval t;
    static struct timeval t_prev;

    gettimeofday(&t, 0);
    float dt = (t.tv_sec - t_prev.tv_sec) + ((float)(t.tv_usec - t_prev.tv_usec)/1000000.0f);
    t_prev = t;

    s_hammer_velocity = (s_hammer_angle - s_hammer_angle_prev) / dt;

    logm(SL4C_DEBUG, "Hammer Velocity = %f r/s", s_hammer_velocity);
}
