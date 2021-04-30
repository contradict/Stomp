#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/select.h>

#include "sensors_control.h"

#include <lcm/lcm.h>

#include "sclog4c/sclog4c.h"

#include "lcm/stomp_control_radio.h"
#include "lcm/stomp_sensors_control.h"

static const string s_hammer_angle_sensor_device = "/sys/bus/iio/devices/iio:device0/in_voltage1_raw";
static const string s_turret_angle_sensor_device = "/sys/bus/iio/devices/iio:device0/in_voltage2_raw";
static const string s_throw_pressure_sensor_device = "/sys/bus/iio/devices/iio:device0/in_voltage3_raw";
static const string s_retract_pressure_sensor_device = "/sys/bus/iio/devices/iio:device0/in_voltage4_raw";

int main(int argc, char **argv)
{
    int opt;
    while((opt = getopt(argc, argv, "d:")) != -1)
    {
        switch(opt)
        {
            case 'd':
                sclog4c_level = atoi(optarg);
                logm(SL4C_FATAL, "Log level set to %d.", sclog4c_level);
                break;
        }
    }

    lcm_t *lcm = lcm_create(NULL);
    if(!lcm)
    {
        logm(SL4C_FATAL, "Failed to initialize LCM.\n");
        exit(2);
    }
    stomp_sensors_control lcm_msg;

    // open each of the analog devices

    int hammer_angle_fd;
    hammer_angle_fd = open(s_hammer_angle_sensor_device, O_RDWR);

    if (hammer_angle_fd < 0)
    {
        logm(SL4C_FATAL, "Error %i from open(): %s", errno, strerror(errno));
        return 1;
    } 

    int turret_angle_fd;
    turret_angle_fd = open(s_turret_angle_sensor_device, O_RDWR);

    if (turret_angle_fd < 0)
    {
        logm(SL4C_FATAL, "Error %i from open(): %s", errno, strerror(errno));
        return 1;
    } 

    int throw_pressure_fd;
    throw_pressure_fd = open(s_throw_pressure_sensor_device, O_RDWR);

    if (throw_pressure_fd < 0)
    {
        logm(SL4C_FATAL, "Error %i from open(): %s", errno, strerror(errno));
        return 1;
    } 

    int retract_pressure_fd;
    retract_pressure_fd = open(s_retract_pressure_sensor_device, O_RDWR);

    if (retract_pressure_fd < 0)
    {
        logm(SL4C_FATAL, "Error %i from open(): %s", errno, strerror(errno));
        return 1;
    } 

    // Read Sensor Inputs, process, and then publish to LCM

    while(true)
    {
        stomp_sensors_controller_publish(lcm, SENSORS_CONTROL, &lcm_msg);
    }

    lcm_destroy(lcm);
    return 0;
}
