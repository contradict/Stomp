## Overview

Units and format of the variables used by hammer_controller are farily complex.  The main ARM processor can deal with the values as floating point, but for the PRU, the values need to be be respresented as fixed point values.  In addition, it is always important to make sure it is clear what the units are for each variables.  There are several variables that represent angles and humans like degrees and pressures are often easier as PSI.  However, the internal representation of angles is in radian and and pressure is in kilopascals.  

The purpose of this document is to clearly specify all of the varous units and representations used by the tools and different processors

## Variables in Cosmos

These need to be focued on being human readable

Config Variables are ALSO specified in the hammer_config.toml file (in these unity and format)

| Config Variable             | Units       | format  |
| --------------------------- | ----------- | --------|
| max_throw_angle             | degrees     | integer |
| min_retract_angle           | degrees     | integer |
| break_exit_velocity         | degrees/sec | float   |
| emergency_break_angle       | degrees     | integer |
| valve_change_dt             | seconds     | float   |
| max_throw_pressure_dt       | seconds     | float   |
| max_throw_expand_dt         | seconds     | float   |
| max_retract_pressure_dt     | seconds     | float   |
| max_retract_expand_dt       | seconds     | float   |
| max_retract_break_dt        | seconds     | float   |
| max_retract_settle_dt       | seconds     | float   |

| Sensor Variable             | Units        | format  |
| -------------------         | -----------  | ------- |
| hammer_angle                | degrees      | float   |
| hammer_velocity             | degrees/sec  | float   |
| hammer_energy               | joules       | float   |
| available_break_energy      | joules       | float   |
| turret_angle                | degrees      | float   |
| turret_velocity             | degrees/sec  | float   |
| throw_pressure              | PSI          | integer |
| retract_pressure            | PSI          | integer |

| Control Radio Variable      | Units        | format  |
| -------------------         | -----------  | --------| 
| throw_desired_intensity     | degrees      | integer |
| retract_desired_intensity   | PSI          | integer |

## Variables in ARM processor code
 
| Config Variable             | Units        |  format
| -------------------         | ------------ | --------
| max_throw_angle             | radian       | float   |
| min_retract_angle           | radian       | float   |
| break_exit_velocity         | radian/sec   | float   |
| emergency_break_angle       | radian       | float   |
| valve_change_dt             | microseconds | integer |
| max_throw_pressure_dt       | microseconds | integer |
| max_throw_expand_dt         | microseconds | integer |
| max_retract_pressure_dt     | microseconds | integer |
| max_retract_expand_dt       | microseconds | integer |
| max_retract_break_dt        | microseconds | integer |
| max_retract_settle_dt       | microseconds | integer |

| Sensor Variable             | Units        | format
| -------------------         | -----------  | --------
| hammer_angle                | radian       | float
| hammer_velocity             | radian/sec   | float
| hammer_energy               | joules       | float
| available_break_energy      | joules       | float
| turret_angle                | radian       | float
| turret_velocity             | radian/sec   | float
| throw_pressure              | pascal       | float
| retract_pressure            | pascal       | float

| Control Radio Variable      | Units        | format
| -------------------         | -----------  | --------
| throw_desired_intensity     | radian       | float
| retract_desired_intensity   | pascal       | float

## Variables in PRU processor code
 
| Config Variable             | Units        |  format
| -------------------         | ------------ | --------
| max_throw_angle             | milliradian  | integer 
| min_retract_angle           | milliradian  | integer
| break_exit_velocity         | mrad/sec     | integer
| emergency_break_angle       | milliradian  | integer
| valve_change_dt             | microseconds | integer
| max_throw_pressure_dt       | microseconds | integer
| max_throw_expand_dt         | microseconds | integer
| max_retract_pressure_dt     | microseconds | integer
| max_retract_expand_dt       | microseconds | integer
| max_retract_break_dt        | microseconds | integer
| max_retract_settle_dt       | microseconds | integer

| Sensor Variable             | Units        | format
| -------------------         | -----------  | --------
| hammer_angle                | milliradian  | integer
| hammer_velocity             | mrad/sec     | integer
| hammer_energy               | joules       | integer
| available_break_energy      | joules       | integer
| turret_angle                | milliradian  | integer
| turret_velocity             | mrad/sec     | integer
| throw_pressure              | pascal       | integer
| retract_pressure            | pascal       | integer

| Control Radio Variable      | Units        | format
| -------------------         | -----------  | --------
| throw_desired_intensity     | milliradian  | integer
| retract_desired_intensity   | pascal       | integer


