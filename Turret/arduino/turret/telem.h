#ifndef TELEM_H
#define TELEM_H
#include "leddar_io.h"
#include "autofire.h"
#include "object.h"

enum TelemetryPacketId {
    TLM_ID_HS=1,
    TLM_ID_LEDDAR=2,
    TLM_DBGM_AUTOFIRE=3,
    TLM_ID_TRK=4,
    TLM_ID_AF=5,
    TLM_ID_ACK=6,
    TLM_ID_AAIM=7,
    TLM_ID_DRV=8,
    // =9,
    TLM_ID_SNS=10,
    TLM_ID_SYS=11,
    TLM_ID_SBS=12,
    TLM_ID_DBGM=13,
    TLM_ID_SWG=14,
    TLM_ID_LEDDARV2=15,
    TLM_ID_PWM=16,
    TLM_ID_IMU=17,
    TLM_ID_DMP=18,
    TLM_ID_ORN=19,
    TLM_ID_SRT=20,
    TLM_ID_OBJM=21,
    TLM_ID_OBJC=22,
    TLM_ID_VAC=23,
};

extern uint32_t enabled_telemetry;

#define _LBV(bit) (1L << (bit))

// Forward decls
struct Detection;

bool sendSystemTelem(uint32_t loop_speed_min, uint32_t loop_speed_avg,
                     uint32_t loop_speed_max, uint32_t loop_count,
                     uint16_t leddar_overrun, uint16_t leddar_crc_error,
                     uint16_t sbus_overrun, uint8_t last_command,
                     uint16_t command_overrun, uint16_t invalid_command,
                     uint16_t valid_command);
bool sendSensorTelem(int16_t pressure, uint16_t angle);
bool sendSbusTelem(uint16_t cmd_bitfield, int16_t hammer_intensity, int16_t hammer_distance);
bool sendDebugMessageTelem(const char *msg);
void debug_print(const String &msg);
bool sendLeddarTelem(const Detection (&detections)[LEDDAR_SEGMENTS], unsigned int count);
bool sendSwingTelem(uint16_t datapoints_collected,
                    uint16_t* angle_data,
                    int16_t* pressure_data,
                    uint16_t data_collect_timestep,
                    uint16_t throw_close_timestep,
                    uint16_t vent_open_timestep,
                    uint16_t throw_close_angle,
                    uint16_t start_angle);
bool sendIMUTelem(int16_t (&a)[3], int16_t (&g)[3], int16_t temperature);
bool sendORNTelem(bool stationary, uint8_t orientation, int32_t sum_angular_rate, int16_t total_norm, int16_t cross_norm);
bool sendDMPTelem(size_t fifoCount, uint8_t intStatus, float w, float x, float y, float z);
bool sendTrackingTelemetry(int16_t detection_x,
                           int16_t detection_y,
                           int32_t detection_angle,
                           int32_t detection_radius,
                           int32_t filtered_x,
                           int32_t filtered_vx,
                           int32_t filtered_y,
                           int32_t filtered_vy);
bool sendAutofireTelemetry(enum AutoFireState st, int32_t swing, int32_t x, int32_t y);
bool sendAutoAimTelemetry(int16_t steer_bias, int16_t theta, int16_t vtheta, int16_t r, int16_t vr);
bool sendCommandAcknowledge(uint8_t cmdid, uint16_t valid_commands, uint16_t invalid_commands);
bool isTimeToSendLeddarTelem(uint32_t now);
bool isTimeToSendTelemetry(uint32_t now);
bool isTimeToSendDriveTelemetry(uint32_t now);
void restoreTelemetryParameters(void);
void setTelemetryParams(uint32_t telemetry_interval,
                        uint32_t leddar_telemetry_interval,
                        uint32_t drive_telem_interval,
                        uint32_t enabled_telemetry);
bool isTLMEnabled(uint8_t tlm_id);
bool sendObjectsTelemetry(uint8_t num_objects, const Object (&objects)[8]);
bool sendVacuumTelemetry(uint16_t sample_period,
                         uint16_t datapoints_collected,
                         int16_t* left_data,
                         int16_t* right_data);
#endif //TELEM_H
