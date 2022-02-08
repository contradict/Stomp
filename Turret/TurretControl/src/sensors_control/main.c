#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <lcm/lcm.h>
#include <toml.h>

#include "utils/utils.h"
#include "sclog4c/sclog4c.h"

#include "lcm_channels.h"
#include "lcm/stomp_control_radio.h"
#include "lcm/stomp_sensors_control.h"

#include "sensors_control/sensors_control.h"
#include "sensors_control/sensors_control_config.h"
#include "sensors_control/hammer_angle_sensor.h"
#include "sensors_control/turret_angle_sensor.h"
#include "sensors_control/throw_pressure_sensor.h"
#include "sensors_control/retract_pressure_sensor.h"

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

static char *s_config_filename = "../sensors_config.toml";

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

static void init();
static void init_lcm();
static void init_sensors();

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    // parse command line
    
    int opt;
    while((opt = getopt(argc, argv, "c:d:")) != -1)
    {
        switch(opt)
        {
            case 'c':
                s_config_filename = strdup(optarg);
                break;
            case 'd':
                sclog4c_level = atoi(optarg);
                logm(SL4C_FATAL, "Log level set to %d.", sclog4c_level);
                break;
        }
    }

    init();

    // Read Sensor Inputs, process, and then publish to LCM

    int select_status;
    struct timeval timeout = { 0, 10000 };
    char read_buff[256];
    int num_bytes;

    stomp_sensors_control lcm_msg;

    while(true)
    {
        struct timeval now;
        static struct timeval last_send_time;

        gettimeofday(&now, 0);
        int dt = time_diff_msec(last_send_time, now);
        logm(SL4C_FINE, "%d msec since last analog read", dt);
        last_send_time = now;

        lcm_msg.hammer_angle = get_hammer_angle();
        lcm_msg.hammer_velocity = get_hammer_velocity();
        lcm_msg.hammer_energy = get_hammer_energy();
        lcm_msg.available_break_energy = get_available_break_energy();
        lcm_msg.turret_angle = get_turret_angle();
        lcm_msg.turret_velocity = get_turret_velocity();
        lcm_msg.throw_pressure = get_throw_pressure();
        lcm_msg.retract_pressure = get_retract_pressure();

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

                calculate_hammer_values(hammer_angle_raw);
                lcm_msg.turret_angle = get_turret_angle();
                lcm_msg.hammer_velocity = get_hammer_velocity();
                lcm_msg.hammer_energy = get_hammer_energy();
                lcm_msg.available_break_energy = get_available_break_energy();
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

                calculate_turret_values(turret_angle_raw);
                lcm_msg.turret_angle = get_turret_angle();
                lcm_msg.turret_velocity = get_turret_velocity();
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

                calculate_throw_values(throw_pressure_raw);
                lcm_msg.throw_pressure = get_throw_pressure();
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

                calculate_retract_values(retract_pressure_raw);
                lcm_msg.retract_pressure = get_retract_pressure();
            }
        }

        logm(SL4C_DEBUG, "\nSENSOR_CONTROL packet:\n\thammer_angle = %f\n\thammer_velocity = %f\n\tturret_angle = %f\n\tturret_velocity = %f\n\tthrow_pressure = %f\n\tretract_pressure = %f",
            lcm_msg.hammer_angle,
            lcm_msg.hammer_velocity,
            lcm_msg.turret_angle,
            lcm_msg.turret_velocity,
            lcm_msg.throw_pressure,
            lcm_msg.retract_pressure);
       
        stomp_sensors_control_publish(s_lcm, SENSORS_CONTROL, &lcm_msg);
    }

    lcm_destroy(s_lcm);
    return 0;
}

void init()
{
    //
    // Read TOML config file
    //

    FILE* fp;
    char errbuf[200];
    if (0 == (fp = fopen(s_config_filename, "r")))
    {
        snprintf(errbuf, sizeof(errbuf),"Unable to open config file %s:", s_config_filename);
        perror(errbuf);
        exit(1);
    }

    toml_table_t *sensors_config = toml_parse_file(fp, errbuf, sizeof(errbuf));

    if (sensors_config == 0)
    {
        logm(SL4C_FATAL, "Unable to parse %s: %s", s_config_filename, errbuf);
        exit(1);
    }

    if (parse_toml_config(sensors_config) < 0)
    {
        logm(SL4C_FATAL, "Unable to parse %s", s_config_filename);
        exit(1);
    }

    //
    // Init all subsystems
    //

    init_lcm();
    init_sensors();
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

