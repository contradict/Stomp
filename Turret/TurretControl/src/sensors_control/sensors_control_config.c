#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include "sclog4c/sclog4c.h"

#include "sensors_control/sensors_control.h"
#include "sensors_control/sensors_control_config.h"
#include "sensors_control/hammer_angle_sensor.h"
#include "sensors_control/turret_angle_sensor.h"
#include "sensors_control/throw_pressure_sensor.h"
#include "sensors_control/retract_pressure_sensor.h"

int parse_toml_config(toml_table_t *toml_config)
{
    logm(SL4C_DEBUG, "Parse toml table into sensor config variables");

    char* dev_scale_device;
    float dev_scale;
    double volt_scale;

    int hammer_angle_sensor_present; 
    char * hammer_angle_sensor_device;
    int64_t hammer_angle_sensor_min_angle_deg;
    double hammer_angle_sensor_min_angle_volts;
    int64_t hammer_angle_sensor_max_angle_deg;
    double hammer_angle_sensor_max_angle_volts;

    int turret_angle_sensor_present; 
    char * turret_angle_sensor_device;
    int64_t turret_angle_sensor_min_angle_deg;
    double turret_angle_sensor_min_angle_volts;
    int64_t turret_angle_sensor_max_angle_deg;
    double turret_angle_sensor_max_angle_volts;

    int throw_pressure_sensor_present; 
    char * throw_pressure_sensor_device;
    int64_t throw_pressure_sensor_min_pressure_psi;
    double throw_pressure_sensor_min_pressure_volts;
    int64_t throw_pressure_sensor_max_pressure_psi;
    double throw_pressure_sensor_max_pressure_volts;

    int retract_pressure_sensor_present; 
    char * retract_pressure_sensor_device;
    int64_t retract_pressure_sensor_min_pressure_psi;
    double retract_pressure_sensor_min_pressure_volts;
    int64_t retract_pressure_sensor_max_pressure_psi;
    double retract_pressure_sensor_max_pressure_volts;

    //
    //  Over All Sensors Config
    //

    toml_table_t *sensors_config_table = toml_table_in(toml_config, "sensors_config");

    toml_raw_t tomlr = toml_raw_in(sensors_config_table, "cape_analog_opamp_scale");
    if (tomlr == 0) { return -1; }
    if (toml_rtod(tomlr, &volt_scale) != 0) { return -1; }

    tomlr = toml_raw_in(sensors_config_table, "sensor_scale_device");
    if (tomlr == 0) { return -1; }
    if (toml_rtos(tomlr, &dev_scale_device) != 0) { return -1;}

    //
    //  Hammer Angle Sensor
    //

    toml_table_t *hammer_angle_sensor_table = toml_table_in(sensors_config_table, "hammer_angle_sensor");

    tomlr = toml_raw_in(hammer_angle_sensor_table, "hammer_angle_sensor_present");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'hammer_angle_sensor_present'"); return -1; }
    if (toml_rtob(tomlr, &hammer_angle_sensor_present) != 0) { return -1; }

    tomlr = toml_raw_in(hammer_angle_sensor_table, "hammer_angle_sensor_device");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'hammer_angle_sensor_device'"); return -1; }
    if (toml_rtos(tomlr, &hammer_angle_sensor_device) != 0) { return -1; }

    tomlr = toml_raw_in(hammer_angle_sensor_table, "hammer_angle_min_degrees");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'hammer_angle_min_degrees'"); return -1; }
    if (toml_rtoi(tomlr, &hammer_angle_sensor_min_angle_deg) != 0) { return -1; }

    tomlr = toml_raw_in(hammer_angle_sensor_table, "hammer_angle_min_sensor_volts");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'hammer_angle_min_sensor_volts'"); return -1; }
    if (toml_rtod(tomlr, &hammer_angle_sensor_min_angle_volts) != 0) { return -1; }

    tomlr = toml_raw_in(hammer_angle_sensor_table, "hammer_angle_max_degrees");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'hammer_angle_max_degrees'"); return -1; }
    if (toml_rtoi(tomlr, &hammer_angle_sensor_max_angle_deg) != 0) { return -1; }

    tomlr = toml_raw_in(hammer_angle_sensor_table, "hammer_angle_max_sensor_volts");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'hammer_angle_max_sensor_volts'"); return -1; }
    if (toml_rtod(tomlr, &hammer_angle_sensor_max_angle_volts) != 0) { return -1; }

    //
    //  Turret Angle Sensor
    //

    toml_table_t *turret_angle_sensor_table = toml_table_in(sensors_config_table, "turret_angle_sensor");

    tomlr = toml_raw_in(turret_angle_sensor_table, "turret_angle_sensor_present");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'turret_angle_sensor_present'"); return -1; }
    if (toml_rtob(tomlr, &turret_angle_sensor_present) != 0) { return -1; }

    tomlr = toml_raw_in(turret_angle_sensor_table, "turret_angle_sensor_device");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'turret_angle_sensor_device'"); return -1; }
    if (toml_rtos(tomlr, &turret_angle_sensor_device) != 0) { return -1; }

    tomlr = toml_raw_in(turret_angle_sensor_table, "turret_angle_min_degrees");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'turret_angle_min_degrees'"); return -1; }
    if (toml_rtoi(tomlr, &turret_angle_sensor_min_angle_deg) != 0) { return -1; }

    tomlr = toml_raw_in(turret_angle_sensor_table, "turret_angle_min_sensor_volts");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'turret_angle_min_sensor_volts'"); return -1; }
    if (toml_rtod(tomlr, &turret_angle_sensor_min_angle_volts) != 0) { return -1; }

    tomlr = toml_raw_in(turret_angle_sensor_table, "turret_angle_max_degrees");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'turret_angle_max_degrees'"); return -1; }
    if (toml_rtoi(tomlr, &turret_angle_sensor_max_angle_deg) != 0) { return -1; }

    tomlr = toml_raw_in(turret_angle_sensor_table, "turret_angle_max_sensor_volts");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'turret_angle_max_sensor_volts'"); return -1; }
    if (toml_rtod(tomlr, &turret_angle_sensor_max_angle_volts) != 0) { return -1; }

    //
    //  Throw Pressure Sensor
    //

    toml_table_t *throw_pressure_sensor_table = toml_table_in(sensors_config_table, "throw_pressure_sensor");

    tomlr = toml_raw_in(throw_pressure_sensor_table, "throw_pressure_sensor_present");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'throw_pressure_sensor_present'"); return -1; }
    if (toml_rtob(tomlr, &throw_pressure_sensor_present) != 0) { return -1; }

    tomlr = toml_raw_in(throw_pressure_sensor_table, "throw_pressure_sensor_device");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'throw_pressure_sensor_device'"); return -1; }
    if (toml_rtos(tomlr, &throw_pressure_sensor_device) != 0) { return -1; }

    tomlr = toml_raw_in(throw_pressure_sensor_table, "throw_pressure_min_psi");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'throw_pressure_min_psi'"); return -1; }
    if (toml_rtoi(tomlr, &throw_pressure_sensor_min_pressure_psi) != 0) { return -1; }

    tomlr = toml_raw_in(throw_pressure_sensor_table, "throw_pressure_min_sensor_volts");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'throw_pressure_min_sensor_volts'"); return -1; }
    if (toml_rtod(tomlr, &throw_pressure_sensor_min_pressure_volts) != 0) { return -1; }

    tomlr = toml_raw_in(throw_pressure_sensor_table, "throw_pressure_max_psi");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'throw_pressure_max_psi'"); return -1; }
    if (toml_rtoi(tomlr, &throw_pressure_sensor_max_pressure_psi) != 0) { return -1; }

    tomlr = toml_raw_in(throw_pressure_sensor_table, "throw_pressure_max_sensor_volts");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'throw_pressure_max_sensor_volts'"); return -1; }
    if (toml_rtod(tomlr, &throw_pressure_sensor_max_pressure_volts) != 0) { return -1; }

    //
    //  Retract Pressure Sensor
    //

    toml_table_t *retract_pressure_sensor_table = toml_table_in(sensors_config_table, "retract_pressure_sensor");

    tomlr = toml_raw_in(retract_pressure_sensor_table, "retract_pressure_sensor_present");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'retract_pressure_sensor_present'"); return -1; }
    if (toml_rtob(tomlr, &retract_pressure_sensor_present) != 0) { return -1; }

    tomlr = toml_raw_in(retract_pressure_sensor_table, "retract_pressure_sensor_device");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'retract_pressure_sensor_device'"); return -1; }
    if (toml_rtos(tomlr, &retract_pressure_sensor_device) != 0) { return -1; }

    tomlr = toml_raw_in(retract_pressure_sensor_table, "retract_pressure_min_psi");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'retract_pressure_min_psi'"); return -1; }
    if (toml_rtoi(tomlr, &retract_pressure_sensor_min_pressure_psi) != 0) { return -1; }

    tomlr = toml_raw_in(retract_pressure_sensor_table, "retract_pressure_min_sensor_volts");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'retract_pressure_min_sensor_volts'"); return -1; }
    if (toml_rtod(tomlr, &retract_pressure_sensor_min_pressure_volts) != 0) { return -1; }

    tomlr = toml_raw_in(retract_pressure_sensor_table, "retract_pressure_max_psi");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'retract_pressure_max_psi'"); return -1; }
    if (toml_rtoi(tomlr, &retract_pressure_sensor_max_pressure_psi) != 0) { return -1; }

    tomlr = toml_raw_in(retract_pressure_sensor_table, "retract_pressure_max_sensor_volts");
    if (tomlr == 0) { logm(SL4C_FATAL, "Error getting 'retract_pressure_max_sensor_volts'"); return -1; }
    if (toml_rtod(tomlr, &retract_pressure_sensor_max_pressure_volts) != 0) { return -1; }

    //
    // Get the voltage scale from the 'sensor_scale_device'
    //

    logm(SL4C_DEBUG, "Open the Analog Device Scale file and read scale value");

    int sensor_scale_fd = open(dev_scale_device, O_RDONLY);
    char read_buff[256];

    if (sensor_scale_fd < 0)
    {
        logm(SL4C_FATAL, "Error %i from open(): %s", errno, strerror(errno));
        return -1;
    } 

    int num_bytes = read(sensor_scale_fd, &read_buff, sizeof(read_buff));
    if (num_bytes <= 0)
    {
        logm(SL4C_FATAL, "Error could not read analog scale value");
        return -1;
    } 

    dev_scale = atof(read_buff);

    //
    // Initialize all sensors with conifig values
    //

    hammer_angle_sensor_init(
        (int8_t)hammer_angle_sensor_present, 
        hammer_angle_sensor_device,
        (int32_t)hammer_angle_sensor_min_angle_deg,
        (float)hammer_angle_sensor_min_angle_volts,
        (int32_t)hammer_angle_sensor_max_angle_deg,
        (float)hammer_angle_sensor_max_angle_volts,
        dev_scale, 
        (float)volt_scale);

    turret_angle_sensor_init(
        (int8_t)turret_angle_sensor_present, 
        turret_angle_sensor_device,
        (int32_t)turret_angle_sensor_min_angle_deg,
        (float)turret_angle_sensor_min_angle_volts,
        (int32_t)turret_angle_sensor_max_angle_deg,
        (float)turret_angle_sensor_max_angle_volts,
        dev_scale, 
        (float)volt_scale);

    throw_pressure_sensor_init(
        (int8_t)throw_pressure_sensor_present, 
        throw_pressure_sensor_device,
        (int32_t)throw_pressure_sensor_min_pressure_psi,
        (float)throw_pressure_sensor_min_pressure_volts,
        (int32_t)throw_pressure_sensor_max_pressure_psi,
        (float)throw_pressure_sensor_max_pressure_volts,
        dev_scale, 
        (float)volt_scale);

    retract_pressure_sensor_init(
        (int8_t)retract_pressure_sensor_present, 
        retract_pressure_sensor_device,
        (int32_t)retract_pressure_sensor_min_pressure_psi,
        (float)retract_pressure_sensor_min_pressure_volts,
        (int32_t)retract_pressure_sensor_max_pressure_psi,
        (float)retract_pressure_sensor_max_pressure_volts,
        dev_scale, 
        (float)volt_scale);

    return 0;
}
