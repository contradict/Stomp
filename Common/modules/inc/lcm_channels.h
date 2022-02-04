#pragma once

// Data received by the sbus radio, uses stomp_control_radio messages
const char* SBUS_RADIO_COMMAND="SBUS_COMMAND";

#ifdef HULL

const char* ADC_OUTPUT="ADC_OUTPUT";
const char* LEG_TELEMETRY="LEG_TELEMETRY";
const char* LEG_COMMAND="LEG_COMMAND";
const char* LEG_RESPONSE="LEG_RESPONSE";

#endif

#ifdef TURRET

const char* SENSORS_CONTROL="SENSORS_CONTROL";
const char* TURRET_TELEMETRY="TURRET_TELEMETRY";
const char* HAMMER_TRIGGER="HAMMER_TRIGGER";
const char* HAMMER_SWING="HAMMER_SWING";
const char* TLM_CMD_HAMMER_CONF="TLM_CMD_HAMMER_CONF";
#endif
