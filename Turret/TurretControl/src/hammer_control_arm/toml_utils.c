#include <stdlib.h>
#include <string.h>

#include "sclog4c/sclog4c.h"

#include "hammer_control_arm/hammer_control_arm.h"
#include "hammer_control_arm/toml_utils.h"

int parse_toml_pru_config(toml_table_t *toml_pru_config)
{
    logm(SL4C_DEBUG, "Parse toml table into pru_config struct");

    toml_raw_t tomlr = toml_raw_in(toml_pru_config, "max_throw_angle");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &g_pru_config.max_throw_angle) != 0)
    {
        return -1;
    }

    tomlr = toml_raw_in(toml_pru_config, "min_retract_angle");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &g_pru_config.min_retract_angle) != 0)
    {
        return -1;
    }

    tomlr = toml_raw_in(toml_pru_config, "min_retract_fill_pressure");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &g_pru_config.min_retract_fill_pressure) != 0)
    {
        return -1;
    }

    tomlr = toml_raw_in(toml_pru_config, "break_exit_velocity");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &g_pru_config.break_exit_velocity) != 0)
    {
        return -1;
    }

    tomlr = toml_raw_in(toml_pru_config, "emergency_break_angle");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &g_pru_config.emergency_break_angle) != 0)
    {
        return -1;
    }

    tomlr = toml_raw_in(toml_pru_config, "valve_change_dt");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &g_pru_config.valve_change_dt) != 0)
    {
        return -1;
    }

    toml_table_t *state_time_limits_config = toml_table_in(toml_pru_config, "state_time_limits");

    tomlr = toml_raw_in(state_time_limits_config, "max_throw_pressure_dt");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &g_pru_config.max_throw_pressure_dt) != 0)
    {
        return -1;
    }

    tomlr = toml_raw_in(state_time_limits_config, "max_throw_expand_dt");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &g_pru_config.max_throw_expand_dt) != 0)
    {
        return -1;
    }

    tomlr = toml_raw_in(state_time_limits_config, "max_retract_pressure_dt");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &g_pru_config.max_retract_pressure_dt) != 0)
    {
        return -1;
    }

    tomlr = toml_raw_in(state_time_limits_config, "max_retract_expand_dt");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &g_pru_config.max_retract_expand_dt) != 0)
    {
        return -1;
    }

    tomlr = toml_raw_in(state_time_limits_config, "max_retract_break_dt");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &g_pru_config.max_retract_break_dt) != 0)
    {
        return -1;
    }

    tomlr = toml_raw_in(state_time_limits_config, "max_retract_settle_dt");
    if (tomlr == 0)
    {
        return -1;
    }
    if (toml_rtoi(tomlr, &g_pru_config.max_retract_settle_dt) != 0)
    {
        return -1;
    }

    return 0;
}
