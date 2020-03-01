#include "Arduino.h"
#include "telem.h"
#include "xbee.h"
#include "leddar_io.h"
#include "pins.h"
#include "DMASerial.h"
#include "utils.h"

static void saveTelemetryParmeters(void);

extern DMASerial& Xbee;

const uint16_t TLM_TERMINATOR=0x6666;

#define CHECK_ENABLED(TLM_ID) if(!(params.enabled_telemetry & _LBV(TLM_ID))) return false;

struct TelemetryParameters {
    uint32_t telemetry_interval;
    uint32_t leddar_telemetry_interval;
    uint32_t drive_telem_interval;
    uint32_t enabled_telemetry;
} __attribute__((packed));

static struct TelemetryParameters EEMEM saved_params = {
    .telemetry_interval=100000L,
    .leddar_telemetry_interval=100000L,
    .drive_telem_interval=500000L,
    .enabled_telemetry=(
            _LBV(TLM_ID_SBS)|
            _LBV(TLM_ID_SRT)|
            _LBV(TLM_ID_TRK)|
            _LBV(TLM_ID_AF)|
            _LBV(TLM_ID_ACK)
            )
};

static struct TelemetryParameters params;

static uint32_t last_telem_time = micros();
static uint32_t last_drive_telem_time = micros();
static uint32_t last_leddar_telem_time = micros();

template <uint8_t packet_id, typename packet_inner> struct TelemetryPacket{
    uint8_t pkt_id;
    packet_inner inner;
    uint16_t terminator;
    TelemetryPacket() : pkt_id(packet_id), terminator(TLM_TERMINATOR) {};
} __attribute__((packed));


struct SystemTelemetryInner {
    uint8_t  weapons_enabled:1;
    uint32_t loop_speed_min;
    uint32_t loop_speed_avg;
    uint32_t loop_speed_max;
    uint32_t loop_count;
    uint16_t leddar_overrun;
    uint16_t leddar_crc_error;
    uint16_t sbus_overrun;
    uint8_t last_command;
    uint16_t command_overrun;
    uint16_t invalid_command;
    uint16_t valid_command;
    uint32_t system_time;
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_SYS, SystemTelemetryInner> SystemTelemetry;

bool sendSystemTelem(uint32_t loop_speed_min, uint32_t loop_speed_avg,
                     uint32_t loop_speed_max, uint32_t loop_count,
                     uint16_t leddar_overrun, uint16_t leddar_crc_error,
                     uint16_t sbus_overrun, uint8_t last_command,
                     uint16_t command_overrun, uint16_t invalid_command,
                     uint16_t valid_command){
    CHECK_ENABLED(TLM_ID_SYS);
    SystemTelemetry tlm;
    tlm.inner.weapons_enabled = g_enabled;
    tlm.inner.loop_speed_min = loop_speed_min;
    tlm.inner.loop_speed_avg = loop_speed_avg;
    tlm.inner.loop_speed_max = loop_speed_max;
    tlm.inner.loop_count = loop_count;
    tlm.inner.leddar_overrun = leddar_overrun;
    tlm.inner.leddar_crc_error = leddar_crc_error;
    tlm.inner.sbus_overrun = sbus_overrun;
    tlm.inner.last_command = last_command;
    tlm.inner.command_overrun = command_overrun;
    tlm.inner.invalid_command = invalid_command;
    tlm.inner.valid_command = valid_command;
    tlm.inner.system_time = millis();
    return Xbee.write((unsigned char *)&tlm, sizeof(tlm));
}

struct SensorTelemetryInner {
    uint16_t pressure;
    uint16_t angle;
    int16_t vacuum_left;
    int16_t vacuum_right;
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_SNS, SensorTelemetryInner> SensorTelemetry;

bool sendSensorTelem(int16_t pressure, uint16_t angle){
    CHECK_ENABLED(TLM_ID_SNS);
    SensorTelemetry tlm;
    tlm.inner.pressure = pressure;
    tlm.inner.angle = angle;
    tlm.inner.vacuum_left = 0;
    tlm.inner.vacuum_right = 0;
    return Xbee.write((unsigned char *)&tlm, sizeof(tlm));
}


struct SBusTelemetryInner {
    uint16_t bitfield;
    int16_t hammer_intensity;
    int16_t hammer_distance;
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_SBS, SBusTelemetryInner> SBusTelemetry;

bool sendSbusTelem(uint16_t cmd_bitfield, int16_t hammer_intensity, int16_t hammer_distance) {
    CHECK_ENABLED(TLM_ID_SBS);
    SBusTelemetry tlm;
    tlm.inner.bitfield = cmd_bitfield;
    tlm.inner.hammer_intensity = hammer_intensity;
    tlm.inner.hammer_distance = hammer_distance;
    return Xbee.write((unsigned char *)&tlm, sizeof(tlm));
}


const size_t MAX_DEBUG_MSG_LENGTH=128;
bool sendDebugMessageTelem(const char *msg){
    CHECK_ENABLED(TLM_ID_DBGM);
    unsigned char pkt[1+MAX_DEBUG_MSG_LENGTH+2]={0};
    pkt[0] = TLM_ID_DBGM;
    size_t copied = 0;
    size_t pos=1;
    while(copied<MAX_DEBUG_MSG_LENGTH && msg[copied]) {
        pkt[pos++] = msg[copied++];
    }
    *((uint16_t *)(pkt+pos)) = TLM_TERMINATOR;
    size_t sendlen = 1+copied+sizeof(TLM_TERMINATOR);
    return Xbee.write(pkt, sendlen);
}

void debug_print(const String &msg){
    sendDebugMessageTelem(msg.c_str());
}

bool isTimeToSendTelemetry(uint32_t now) {
    bool send = now - last_telem_time > params.telemetry_interval;
    if(send) {
        last_telem_time = now;
    }
    return send;
}

struct LeddarTelemetryInner {
    uint16_t state;
    uint16_t count;
    uint16_t range[LEDDAR_SEGMENTS];
    uint16_t amplitude[LEDDAR_SEGMENTS];
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_LEDDARV2, LeddarTelemetryInner> LeddarTelemetry;

static LeddarTelemetry leddar_tlm;
bool sendLeddarTelem(const Detection (&min_detections)[LEDDAR_SEGMENTS], unsigned int count){
  CHECK_ENABLED(TLM_ID_LEDDARV2);
  leddar_tlm.inner.count = count;
  for (uint8_t i = 0; i < LEDDAR_SEGMENTS; i++){
      leddar_tlm.inner.range[i] = min_detections[i].Distance;
      leddar_tlm.inner.amplitude[i] = min_detections[i].Amplitude;
  }
  return Xbee.enqueue((unsigned char *)&leddar_tlm, sizeof(leddar_tlm),
                      NULL, NULL);
}
bool isTimeToSendLeddarTelem(uint32_t now) {
    bool send = now - last_leddar_telem_time > params.leddar_telemetry_interval;
    if(send) {
        last_leddar_telem_time = now;
    }
    return send;
}


struct SwingTelemInner {
    uint16_t sample_period;
    uint16_t throw_close;
    uint16_t vent_open;
    uint16_t throw_close_angle;
    uint16_t start_angle;
    uint16_t datapoints;
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_SWG, SwingTelemInner> SwingTelemetry;

bool sendSwingTelem(uint16_t datapoints_collected,
                    uint16_t* angle_data,
                    int16_t* pressure_data,
                    uint16_t data_collect_timestep,
                    uint16_t throw_close_timestep,
                    uint16_t vent_open_timestep,
                    uint16_t throw_close_angle,
                    uint16_t start_angle) {
    CHECK_ENABLED(TLM_ID_SWG);
    SwingTelemetry tlm;
    tlm.inner.sample_period = data_collect_timestep;
    tlm.inner.throw_close = throw_close_timestep;
    tlm.inner.vent_open = vent_open_timestep;
    tlm.inner.throw_close_angle = throw_close_angle;
    tlm.inner.start_angle = start_angle;
    tlm.inner.datapoints = datapoints_collected;
    bool success = Xbee.write((unsigned char *)&tlm, sizeof(tlm)-sizeof(TLM_TERMINATOR));
    if(success)
    {
        success &= Xbee.enqueue((uint8_t *)angle_data, sizeof(uint16_t)*256, NULL, NULL);
        success &= Xbee.enqueue((uint8_t *)pressure_data, sizeof(int16_t)*256, NULL, NULL);
    }
    success &= Xbee.write((uint8_t *)&tlm.terminator, sizeof(tlm.terminator));

    return success;
}

struct IMUTelemInner {
    int16_t a[3];
    int16_t g[3];
    int16_t t;
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_IMU, IMUTelemInner> IMUTelemetry;

bool sendIMUTelem(int16_t (&a)[3], int16_t (&g)[3], int16_t t)
{
    CHECK_ENABLED(TLM_ID_IMU);
    IMUTelemetry tlm;
    for(size_t i=0;i<3;i++) {
        tlm.inner.a[i] = a[i];
        tlm.inner.g[i] = g[i];
    }
    tlm.inner.t = t;
    return Xbee.write((unsigned char *)&tlm, sizeof(tlm));
}

struct DMPTelemInner {
    uint16_t fifoCount;
    uint8_t intStatus;
    float qw, qx, qy, qz;
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_DMP, DMPTelemInner> DMPTelemetry;

bool sendDMPTelem(size_t fifoCount, uint8_t intStatus, float w, float x, float y, float z)
{
    CHECK_ENABLED(TLM_ID_DMP);
    DMPTelemetry tlm;
    tlm.inner.fifoCount = fifoCount;
    tlm.inner.intStatus = intStatus;
    tlm.inner.qw = w;
    tlm.inner.qx = x;
    tlm.inner.qy = y;
    tlm.inner.qz = z;
    return Xbee.write((unsigned char *)&tlm, sizeof(tlm));
}

struct ORNTelemInner {
    uint8_t padding:3;
    uint8_t orientation:4;
    uint8_t stationary:1;
    int32_t sum_angular_rate;
    int16_t total_norm;
    int16_t cross_norm;
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_ORN, ORNTelemInner> ORNTelemetry;
bool sendORNTelem(bool stationary, uint8_t orientation, int32_t sum_angular_rate, int16_t total_norm, int16_t cross_norm)
{
    CHECK_ENABLED(TLM_ID_ORN);
    ORNTelemetry tlm;
    tlm.inner.stationary = stationary;
    tlm.inner.orientation = orientation;
    tlm.inner.sum_angular_rate = sum_angular_rate;
    tlm.inner.total_norm = total_norm;
    tlm.inner.cross_norm = cross_norm;
    return Xbee.write((unsigned char *)&tlm, sizeof(tlm));
}

bool isTimeToSendDriveTelemetry(uint32_t now) {
    CHECK_ENABLED(TLM_ID_DRV);
    bool send = now-last_drive_telem_time > params.drive_telem_interval;
    if(send) {
        last_drive_telem_time = now;
    }
    return send;
}

struct TrackingTelemetryInner {
    int16_t detection_x;
    int16_t detection_y;
    int32_t detection_angle;
    int32_t detection_radius;
    int16_t filtered_x;
    int16_t filtered_vx;
    int16_t filtered_y;
    int16_t filtered_vy;
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_TRK, TrackingTelemetryInner> TRKTelemetry;
bool sendTrackingTelemetry(int16_t detection_x,
                           int16_t detection_y,
                           int32_t detection_angle,
                           int32_t detection_radius,
                           int32_t filtered_x,
                           int32_t filtered_vx,
                           int32_t filtered_y,
                           int32_t filtered_vy) {
    CHECK_ENABLED(TLM_ID_TRK);
    TRKTelemetry tlm;
    tlm.inner.detection_x = detection_x;
    tlm.inner.detection_y = detection_y;
    tlm.inner.detection_angle = detection_angle;
    tlm.inner.detection_radius = detection_radius;
    tlm.inner.filtered_x = (int16_t)clip(filtered_x, -32768L, 32767L);
    tlm.inner.filtered_vx = (int16_t)clip(filtered_vx, -32768L, 32767L);
    tlm.inner.filtered_y = (int16_t)clip(filtered_y, -32768L, 32767L);
    tlm.inner.filtered_vy = (int16_t)clip(filtered_vy, -32768L, 32767L);
    return Xbee.write((unsigned char *)&tlm, sizeof(tlm));
}


struct AutofireTelemetryInner {
    int8_t state;
    int32_t swing;
    int16_t x;
    int16_t y;
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_AF, AutofireTelemetryInner> AFTelemetry;
bool sendAutofireTelemetry(enum AutoFireState st, int32_t swing, int32_t x, int32_t y) {
    CHECK_ENABLED(TLM_ID_AF);
    AFTelemetry tlm;
    tlm.inner.state = st;
    tlm.inner.swing = swing;
    tlm.inner.x = (int16_t)clip(x, -32768L, 32767L);
    tlm.inner.y = (int16_t)clip(y, -32768L, 32767L);
    return Xbee.write((unsigned char *)&tlm, sizeof(tlm));
}

struct AutoAimTelemetryInner {
    int16_t steer_bias;
    int16_t theta;
    int16_t vtheta;
    int16_t radius;
    int16_t vradius;
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_AAIM, AutoAimTelemetryInner> AAIMTelemetry;
bool sendAutoAimTelemetry(int16_t steer_bias, int16_t theta, int16_t vtheta, int16_t r, int16_t vr) {
    CHECK_ENABLED(TLM_ID_AAIM);
    AAIMTelemetry tlm;
    tlm.inner.steer_bias = steer_bias;
    tlm.inner.theta = theta;
    tlm.inner.vtheta = vtheta;
    tlm.inner.radius = r;
    tlm.inner.vradius = vr;
    return Xbee.write((unsigned char *)&tlm, sizeof(tlm));
}

struct CommandAcknolwedgeInner {
    uint8_t cmdid;
    uint16_t valid;
    uint16_t invalid;
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_ACK, CommandAcknolwedgeInner> ACKTelemetry;
bool sendCommandAcknowledge(uint8_t command, uint16_t valid, uint16_t invalid) {
    CHECK_ENABLED(TLM_ID_ACK);
    ACKTelemetry tlm;
    tlm.inner.cmdid = command;
    tlm.inner.valid = valid;
    tlm.inner.invalid = invalid;
    return Xbee.write((unsigned char *)&tlm, sizeof(tlm));
}

void setTelemetryParams(uint32_t telemetry_interval,
                        uint32_t leddar_telemetry_interval,
                        uint32_t drive_telem_interval,
                        uint32_t enabled_telemetry) {
    params.telemetry_interval = telemetry_interval;
    params.leddar_telemetry_interval = leddar_telemetry_interval;
    params.drive_telem_interval = drive_telem_interval;
    params.enabled_telemetry = enabled_telemetry;
    saveTelemetryParmeters();
}

static void saveTelemetryParmeters(void) {
    eeprom_write_block(&params, &saved_params, sizeof(struct TelemetryParameters));
}

void restoreTelemetryParameters(void) {
    eeprom_read_block(&params, &saved_params, sizeof(struct TelemetryParameters));
}

bool isTLMEnabled(uint8_t tlm_id) {
    CHECK_ENABLED(tlm_id)
    return true;
}

struct ObjectsCalcuatedInner {
   uint8_t num_objects;
   int16_t object_radius[8];
   int16_t object_angle[8];
   int16_t object_x[8];
   int16_t object_y[8];
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_OBJC, ObjectsCalcuatedInner> ObjectsCalculatedTelemetry;
struct ObjectsMeasuredInner {
   uint8_t num_objects;
   uint8_t left_edge[8];
   uint8_t right_edge[8];
   int16_t object_sum_distance[8];
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_OBJM, ObjectsMeasuredInner> ObjectsMeasuredTelemetry;
bool sendObjectsCalculatedTelemetry(uint8_t num_objects, const Object (&objects)[8])
{
    CHECK_ENABLED(TLM_ID_OBJC);
    ObjectsCalculatedTelemetry tlm;
    tlm.inner.num_objects = num_objects;
    int i=0;
    for(; i<num_objects; i++)
    {
        tlm.inner.object_radius[i] = objects[i].radius();
        tlm.inner.object_angle[i] = objects[i].angle();
        tlm.inner.object_x[i] = objects[i].xcoord();
        tlm.inner.object_y[i] = objects[i].ycoord();
    }
    for(; i<8; i++)
    {
        tlm.inner.object_radius[i] = -1;
        tlm.inner.object_angle[i] = -1;
        tlm.inner.object_x[i] = -1;
        tlm.inner.object_y[i] = -1;
    }
    return Xbee.write((unsigned char *)&tlm, sizeof(tlm));
}
bool sendObjectsMeasuredTelemetry(uint8_t num_objects, const Object (&objects)[8])
{
    CHECK_ENABLED(TLM_ID_OBJM);
    ObjectsMeasuredTelemetry tlm;
    tlm.inner.num_objects = num_objects;
    int i=0;
    for(; i<num_objects; i++)
    {
        tlm.inner.left_edge[i] = objects[i].LeftEdge;
        tlm.inner.right_edge[i] = objects[i].RightEdge;
        tlm.inner.object_sum_distance[i] = objects[i].SumDistance;
    }
    for(; i<8; i++)
    {
        tlm.inner.left_edge[i] = -1;
        tlm.inner.right_edge[i] = -1;
        tlm.inner.object_sum_distance[i] = -1;
    }
    return Xbee.write((unsigned char *)&tlm, sizeof(tlm));
}
bool sendObjectsTelemetry(uint8_t num_objects, const Object (&objects)[8])
{
    return (sendObjectsMeasuredTelemetry(num_objects, objects) +
            sendObjectsCalculatedTelemetry(num_objects, objects));
}

struct VacuumTelemInner {
    uint16_t sample_period;
    uint16_t datapoints;
} __attribute__((packed));
typedef TelemetryPacket<TLM_ID_VAC, VacuumTelemInner> VacuumTelemetry;

bool sendVacuumTelemetry(uint16_t sample_period,
                         uint16_t datapoints_collected,
                         int16_t* left_vacuum,
                         int16_t* right_vacuum)
{
    CHECK_ENABLED(TLM_ID_VAC);
    VacuumTelemetry tlm;
    tlm.inner.sample_period = sample_period;
    tlm.inner.datapoints = datapoints_collected;
    bool success = Xbee.write((unsigned char *)&tlm, sizeof(tlm)-sizeof(TLM_TERMINATOR));
    if(success)
    {
        success &= Xbee.enqueue(
            (uint8_t *)left_vacuum, sizeof(uint16_t)*datapoints_collected, NULL, NULL);
        success &= Xbee.enqueue(
            (uint8_t *)right_vacuum, sizeof(int16_t)*datapoints_collected, NULL, NULL);
    }
    success &= Xbee.write((uint8_t *)&tlm.terminator, sizeof(tlm.terminator));

    return success;
}


