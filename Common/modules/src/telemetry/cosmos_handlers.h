#pragma once

// Extens for all the LCM Channels so they can be used by other (than main.c) files.

extern const char* SBUS_RADIO_COMMAND;

void cosmos_handle();

#ifdef HULL
#endif

#ifdef TURRET

extern const char* SENSORS_CONTROL;
extern const char* TURRET_TELEMETRY;
extern const char* HAMMER_TRIGGER;
extern const char* HAMMER_SWING;
extern const char* TLM_CMD_HAMMER_CONF;

typedef struct _cosmos_hammer_conf_msg cosmos_hammer_conf_msg;
struct _cosmos_hammer_conf_msg
{
    int8_t msg_id;
    int32_t max_throw_angle;
    int32_t min_retract_angle;
    float break_exit_velocity;
    int32_t emergency_break_angle;
    float valve_change_dt;
    float max_throw_pressure_dt;
    float max_throw_expand_dt;
    float max_retract_pressure_dt;
    float max_retract_expand_dt;
    float max_retract_break_dt;
    float max_retract_settle_dt;
};

void hammer_conf_handler(char* message);
#endif