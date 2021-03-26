#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <lcm/lcm.h>

#include "sclog4c/sclog4c.h"

#include "lcm_channels.h"
#include "lcm/stomp_control_radio.h"
#include "lcm/stomp_sensors_control.h"

#include "sensors_control/sensors_control.h"
#include "sensors_control/hammer_angle_sensor.h"
#include "sensors_control/turret_angle_sensor.h"
#include "sensors_control/throw_pressure_sensor.h"
#include "sensors_control/retract_pressure_sensor.h"

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static lcm_t *s_lcm;

static int s_hammer_angle_fd = -1;
static int s_turret_angle_fd = -1;
static int s_throw_pressure_fd = -1;
static int s_retract_pressure_fd = -1;
static int s_max_fd = -1;


// -----------------------------------------------------------------------------
//  forward decl of internal methods
// -----------------------------------------------------------------------------

static void init_lcm();
static void init_sensors();

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    // parse command line
    
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

    init_lcm();
    init_sensors();

    // Read Sensor Inputs, process, and then publish to LCM

    int select_status;
    struct timeval timeout = { 0, 10000 };
    char read_buff[256];
    int num_bytes;

    stomp_sensors_control lcm_msg;

    while(true)
    {
        lcm_msg.hammer_angle = 0;
        lcm_msg.hammer_angle_valid = 0;
        lcm_msg.turret_angle = 0;
        lcm_msg.turret_angle_valid = 0;
        lcm_msg.throw_pressure = 0;
        lcm_msg.throw_pressure_valid = 0;
        lcm_msg.retract_pressure = 0;
        lcm_msg.retract_pressure_valid = 0;

        fd_set fds;
        FD_ZERO(&fds);
        if (s_hammer_angle_fd >= 0) FD_SET(s_hammer_angle_fd, &fds);
        if (s_turret_angle_fd >= 0) FD_SET(s_turret_angle_fd, &fds);
        if (s_throw_pressure_fd >= 0) FD_SET(s_throw_pressure_fd, &fds);
        if (s_retract_pressure_fd >= 0) FD_SET(s_retract_pressure_fd, &fds);

        select_status = select(s_max_fd, &fds, NULL, NULL, &timeout);

        if (select_status > 0)
        {
            // read and process hammer angle

            if (FD_ISSET(s_hammer_angle_fd, &fds))
            {
                memset(&read_buff, '\0', sizeof(read_buff));
                num_bytes = read(s_hammer_angle_fd, &read_buff, sizeof(read_buff));
                logm(SL4C_FINE, "read s_hammer_angle_fd returned %d bytes", num_bytes);
                lseek(s_hammer_angle_fd, 0, SEEK_SET);

                int32_t hammer_angle_raw = atoi(read_buff);
                logm(SL4C_FINE, "hammer angle raw = %d", hammer_angle_raw);

                lcm_msg.hammer_angle_valid = 1;
                lcm_msg.hammer_angle = process_raw_hammer_angle(hammer_angle_raw);
            }

            // read and process turret angle
            
            if (FD_ISSET(s_turret_angle_fd, &fds))
            {
                memset(&read_buff, '\0', sizeof(read_buff));
                num_bytes = read(s_turret_angle_fd, &read_buff, sizeof(read_buff));
                logm(SL4C_FINE, "read s_turret_angle_fd returned %d bytes", num_bytes);
                lseek(s_turret_angle_fd, 0, SEEK_SET);

                int32_t turret_angle_raw = atoi(read_buff);
                logm(SL4C_FINE, "turret angle raw = %d", turret_angle_raw);

                lcm_msg.turret_angle_valid = 1;
                lcm_msg.turret_angle = process_raw_turret_angle(turret_angle_raw);
            }

            // read and process throw pressure
            
            if (FD_ISSET(s_throw_pressure_fd, &fds))
            {
                memset(&read_buff, '\0', sizeof(read_buff));
                num_bytes = read(s_throw_pressure_fd, &read_buff, sizeof(read_buff));
                logm(SL4C_FINE, "read s_throw_pressure_fd returned %d bytes", num_bytes);
                lseek(s_throw_pressure_fd, 0, SEEK_SET);

                int32_t throw_pressure_raw = atoi(read_buff);
                logm(SL4C_FINE, "throw pressure raw = %d", throw_pressure_raw);

                lcm_msg.throw_pressure_valid = 1;
                lcm_msg.throw_pressure = process_raw_throw_pressure(throw_pressure_raw);
            }

            // read and process retract pressure
            
            if (FD_ISSET(s_retract_pressure_fd, &fds))
            {
                memset(&read_buff, '\0', sizeof(read_buff));
                num_bytes = read(s_retract_pressure_fd, &read_buff, sizeof(read_buff));
                logm(SL4C_FINE, "read s_retract_pressure_fd returned %d bytes", num_bytes);
                lseek(s_retract_pressure_fd, 0, SEEK_SET);

                int32_t retract_pressure_raw = atoi(read_buff);
                logm(SL4C_FINE, "retract pressure raw = %d", retract_pressure_raw);

                lcm_msg.retract_pressure_valid = 1;
                lcm_msg.retract_pressure = process_raw_retract_pressure(retract_pressure_raw);
            }
        }

        logm(SL4C_INFO, "\nSENSOR_CONTROL packet:\n\thammer_angle_valid: %d\n\thammer_angle = %f\n\tturret_angle_valid: %d\n\tturret_angle = %f\n\tthrow_pressure_valid: %d\n\tthrow_pressure = %f\n\tretract_pressure_valid: %d\n\tretract_pressure = %f",
            lcm_msg.hammer_angle_valid,
            lcm_msg.hammer_angle,
            lcm_msg.turret_angle_valid,
            lcm_msg.turret_angle,
            lcm_msg.throw_pressure_valid,
            lcm_msg.throw_pressure,
            lcm_msg.retract_pressure_valid,
            lcm_msg.retract_pressure);

        stomp_sensors_control_publish(s_lcm, SENSORS_CONTROL, &lcm_msg);
    }

    lcm_destroy(s_lcm);
    return 0;
}

void init_lcm()
{
    // create lcm 

    s_lcm = lcm_create(NULL);

    if(!s_lcm)
    {
        logm(SL4C_FATAL, "Failed to initialize LCM.\n");
        exit(2);
    }
}

void init_sensors()
{
    // open each of the analog devices
    // 
    // need to keep track of the higest fd for use in select call

    if (is_hammer_angle_sensor_present())
    {
        s_hammer_angle_fd = open(get_hammer_angle_sensor_device(), O_RDONLY);

        if (s_hammer_angle_fd < 0)
        {
            logm(SL4C_FATAL, "Error %i from open(): %s", errno, strerror(errno));
            exit(2);
        } 

        if (s_hammer_angle_fd > s_max_fd)
        {
            s_max_fd = s_hammer_angle_fd;
        }
    }

    if (is_turret_angle_sensor_present())
    {
        s_turret_angle_fd = open(get_hammer_angle_sensor_device(), O_RDONLY);

        if (s_turret_angle_fd < 0)
        {
            logm(SL4C_FATAL, "Error %i from open(): %s", errno, strerror(errno));
            exit(2);
        } 
    
        if (s_turret_angle_fd > s_max_fd)
        {
            s_max_fd = s_turret_angle_fd;
        }
    }

    if (is_throw_pressure_sensor_present())
    {
        s_throw_pressure_fd = open(get_throw_pressure_sensor_device(), O_RDONLY);
    
        if (s_throw_pressure_fd < 0)
        {
            logm(SL4C_FATAL, "Error %i from open(): %s", errno, strerror(errno));
            exit(2);
        } 
    
        if (s_throw_pressure_fd > s_max_fd)
        {
            s_max_fd = s_throw_pressure_fd;
        }
    }

    if (is_retract_pressure_sensor_present())
    {
        s_retract_pressure_fd = open(get_retract_pressure_sensor_device(), O_RDONLY);

        if (s_retract_pressure_fd < 0)
        {
            logm(SL4C_FATAL, "Error %i from open(): %s", errno, strerror(errno));
            exit(2);
        } 
    
        if (s_retract_pressure_fd > s_max_fd)
        {
            s_max_fd = s_retract_pressure_fd;
        }
    }

    //  select takes highest fd + 1 so increment s_max_fd

    if (s_max_fd < 0)
    {
        logm(SL4C_FATAL, "Error: could not open any sensor devices");
        exit(2);
    } 

    s_max_fd++;
}

