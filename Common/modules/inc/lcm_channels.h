#pragma once

const char* SBUS_RADIO_COMMAND="SBUS_COMMAND";

#if HULL

const char* ADC_OUTPUT="ADC_OUTPUT";
const char* LEG_TELEMETRY="LEG_TELEMETRY";
const char* LEG_COMMAND="LEG_COMMAND";
const char* LEG_RESPONSE="LEG_RESPONSE";

#endif

#if TURRET

const char* SENSORS_CONTROL="SENSORS_CONTROL";
const char* TURRET_TELEMETRY="TURRET_TELEMETRY";

#endif
