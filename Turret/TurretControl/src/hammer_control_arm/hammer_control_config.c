#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "sclog4c/sclog4c.h"

#include "hammer_control_arm/hammer_control_arm.h"
#include "hammer_control_arm/hammer_control_config.h"

int parse_toml_pru_config(toml_table_t *toml_pru_config)
{
    logm(SL4C_DEBUG, "Parse toml table into pru_config struct");

    // See HammerControllerValues.md to understand the various
    // units and formats of all variables.  
    //
    // This code will CONVERT values from the human readable 
    // versions used in TOML / COSMOS into the ARM format.
    
    int64_t degrees = 0;
    double degreesPerSecond = 0.0f;
    double seconds = 0.0f;

    toml_raw_t tomlr = toml_raw_in(toml_pru_config, "max_throw_angle");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &degrees) == 0)
    {
        g_pru_config.max_throw_angle = degrees * M_PI/180.0f;
    }
    else 
    {
        return -1;
    }

    tomlr = toml_raw_in(toml_pru_config, "min_retract_angle");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &degrees) == 0)
    {
        g_pru_config.min_retract_angle = degrees * M_PI/180.0f;
    }
    else 
    {
        return -1;
    }

    tomlr = toml_raw_in(toml_pru_config, "break_exit_velocity");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtod(tomlr, &degreesPerSecond) == 0)
    {
        g_pru_config.break_exit_velocity = degreesPerSecond * M_PI/180.0f;
    }
    else
    {
        return -1;
    }

    tomlr = toml_raw_in(toml_pru_config, "emergency_break_angle");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &degrees) == 0)
    {
        g_pru_config.emergency_break_angle = degrees * M_PI/180.0f;
    }
    else
    {
        return -1;
    }

    tomlr = toml_raw_in(toml_pru_config, "valve_change_dt");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtod(tomlr, &seconds) == 0)
    {
        g_pru_config.valve_change_dt = (int32_t)(seconds * 1000000.0f);
    }
    else
    {
        return -1;
    }

    toml_table_t *state_time_limits_config = toml_table_in(toml_pru_config, "state_time_limits");

    tomlr = toml_raw_in(state_time_limits_config, "max_throw_pressure_dt");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtod(tomlr, &seconds) == 0)
    {
        g_pru_config.max_throw_pressure_dt = (int32_t)(seconds * 1000000.0f);
    }
    else
    {
        return -1;
    }

    tomlr = toml_raw_in(state_time_limits_config, "max_throw_expand_dt");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtod(tomlr, &seconds) == 0)
    {
        g_pru_config.max_throw_expand_dt = (int32_t)(seconds * 1000000.0f);
    }
    else
    {
        return -1;
    }

    tomlr = toml_raw_in(state_time_limits_config, "max_retract_pressure_dt");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtod(tomlr, &seconds) == 0)
    {
        g_pru_config.max_retract_pressure_dt = (int32_t)(seconds * 1000000.0f);
    }
    else
    {
        return -1;
    }

    tomlr = toml_raw_in(state_time_limits_config, "max_retract_expand_dt");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtod(tomlr, &seconds) == 0)
    {
        g_pru_config.max_retract_expand_dt = (int32_t)(seconds * 1000000.0f);
    }
    else
    {
        return -1;
    }

    tomlr = toml_raw_in(state_time_limits_config, "max_retract_break_dt");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtod(tomlr, &seconds) == 0)
    {
        g_pru_config.max_retract_break_dt = (int32_t)(seconds * 1000000.0f);
    }
    else
    {
        return -1;
    }

    tomlr = toml_raw_in(state_time_limits_config, "max_retract_settle_dt");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtod(tomlr, &seconds) == 0)
    {
        g_pru_config.max_retract_settle_dt = (int32_t)(seconds * 1000000.0f);
    }
    else
    {
        return -1;
    }

    return 0;
}
