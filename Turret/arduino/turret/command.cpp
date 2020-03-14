#include "command.h"
#include "DMASerial.h"
#include "telem.h"
#include "targeting.h"
#include "autofire.h"
#include "imu.h"
#include "turretController.h"

#define MAXIMUM_COMMAND_LENGTH 64
enum Commands {
    CMD_ID_TRATE = 10,
    CMD_ID_TRKFLT = 11,
    CMD_ID_OBJSEG = 12,
    CMD_ID_AF = 13,
    CMD_ID_AAIM = 14,
    CMD_ID_IMUP = 15,
    CMD_ID_LDDR = 17,
};

extern Track g_trackedObject;

const uint16_t CMD_TERMINATOR=0x6666;

template <uint8_t command_id, typename command_inner> struct CommandPacket{
    uint8_t cmd_id;
    command_inner inner;
    uint16_t terminator;
    CommandPacket() : cmd_id(command_id), terminator(CMD_TERMINATOR) {};
} __attribute__((packed));


struct TelemetryRateInner {
    uint32_t small_telem_period;
    uint32_t leddar_telem_period;
    uint32_t drive_telem_period;
    uint32_t enabled_messages;
} __attribute__((packed));
typedef CommandPacket<CMD_ID_TRATE, TelemetryRateInner> TelemetryRateCommand;


struct ObjectSegmentationInner {
    int16_t min_object_size;
    int16_t max_object_size;
    int16_t edge_call_threshold;
    uint8_t closest_only:1;
} __attribute__((packed));
typedef CommandPacket<CMD_ID_OBJSEG, ObjectSegmentationInner> ObjectSegmentationCommand;


struct AutoFireInner {
    int16_t xtol;
    int16_t ytol;
    int16_t max_omegaZ;
    uint32_t telemetry_interval;
} __attribute__((packed));
typedef CommandPacket<CMD_ID_AF, AutoFireInner> AutoFireCommand;

struct AutoAimInner {
    int32_t steer_p;
    int32_t steer_d;
    int32_t steer_max;
    int32_t gyro_gain;
    uint32_t telemetry_interval;
} __attribute__((packed));
typedef CommandPacket<CMD_ID_AAIM, AutoAimInner> AutoAimCommand;

struct TrackingFilterInner {
    int16_t alpha;
    int16_t beta;
    int8_t min_num_updates;
    uint32_t track_lost_dt;
    int16_t max_off_track;
    int16_t max_start_distance;
} __attribute__((packed));
typedef CommandPacket<CMD_ID_TRKFLT, TrackingFilterInner> TrackingFilterCommand;

struct IMUParameterInner {
    int8_t dlpf;
    uint32_t imu_period;
    int32_t stationary_threshold;
    int16_t upright_cross;
    int16_t min_valid_cross;
    int16_t max_valid_cross;
    int16_t max_total_norm;
    int16_t x_threshold;
    int16_t z_threshold;
} __attribute__((packed));
typedef CommandPacket<CMD_ID_IMUP, IMUParameterInner> IMUParameterCommand;

struct LeddarCommandInner {
    uint16_t min_detection_distance;
    uint16_t max_detection_distance;
} __attribute__((packed));
typedef CommandPacket<CMD_ID_LDDR, LeddarCommandInner> LeddarCommand;

static uint8_t command_buffer[MAXIMUM_COMMAND_LENGTH];
static size_t command_length=0;
static bool command_ready = false;
uint16_t command_overrun = 0;
uint16_t invalid_command = 0;
uint16_t valid_command = 0;
uint8_t last_command = 0;

void serialEvent(void) {
    while(DSerial.available()) {
        uint8_t in = (uint8_t)DSerial.read();
        if(!command_ready &&
            command_length<MAXIMUM_COMMAND_LENGTH) {
            command_buffer[command_length++] = in;
            if(command_length>=MAXIMUM_COMMAND_LENGTH) {
                command_length = 0;
                command_overrun ++;
            }
        }
    }

    if(command_length>2 &&
       command_buffer[command_length-1] == '\x66' &&
       command_buffer[command_length-2] == '\x66') {
        command_ready = true;
    }
}

void handleCommands(void) 
{
    TelemetryRateCommand *trate_cmd;
    TrackingFilterCommand *trkflt_cmd;
    ObjectSegmentationCommand *objseg_cmd;
    AutoFireCommand *af_cmd;
    AutoAimCommand *aaim_cmd;
    IMUParameterCommand *imup_cmd;
    LeddarCommand *leddar_cmd;

    if(command_ready) 
    {
        last_command = command_buffer[0];
        switch(last_command) 
        {
            case CMD_ID_TRATE:
                trate_cmd = (TelemetryRateCommand *)command_buffer;
                setTelemetryParams(trate_cmd->inner.small_telem_period,
                                    trate_cmd->inner.leddar_telem_period,
                                    trate_cmd->inner.drive_telem_period,
                                    trate_cmd->inner.enabled_messages);
                debug_print(String("enabled_telemetry=")+
                            String(trate_cmd->inner.enabled_messages, 16));
                valid_command++;
                break;
            case CMD_ID_TRKFLT:
                trkflt_cmd = (TrackingFilterCommand *)command_buffer;
                g_trackedObject.setTrackingFilterParams(trkflt_cmd->inner.alpha,
                                        trkflt_cmd->inner.beta,
                                        trkflt_cmd->inner.min_num_updates,
                                        trkflt_cmd->inner.track_lost_dt,
                                        trkflt_cmd->inner.max_off_track,
                                        trkflt_cmd->inner.max_start_distance);
                valid_command++;
                break;
            case CMD_ID_OBJSEG:
                objseg_cmd = (ObjectSegmentationCommand *)command_buffer;
                setObjectSegmentationParams(
                                        objseg_cmd->inner.min_object_size,
                                        objseg_cmd->inner.max_object_size,
                                        objseg_cmd->inner.edge_call_threshold,
                                        objseg_cmd->inner.closest_only);
                valid_command++;
                break;
            case CMD_ID_AF:
                af_cmd = (AutoFireCommand *)command_buffer;
                setAutoFireParams(af_cmd->inner.xtol,
                                af_cmd->inner.ytol,
                                af_cmd->inner.max_omegaZ,
                                af_cmd->inner.telemetry_interval);
                valid_command++;
                break;
            case CMD_ID_AAIM:
                aaim_cmd = (AutoAimCommand *)command_buffer;
                Turret.SetAutoAimParameters(
                    aaim_cmd->inner.steer_p,
                    aaim_cmd->inner.steer_d,
                    aaim_cmd->inner.steer_max,
                    aaim_cmd->inner.gyro_gain,
                    aaim_cmd->inner.telemetry_interval);
                break;
            case CMD_ID_IMUP:
                imup_cmd = (IMUParameterCommand *)command_buffer;
                setIMUParameters(
                    imup_cmd->inner.dlpf, imup_cmd->inner.imu_period,
                    imup_cmd->inner.stationary_threshold,
                    imup_cmd->inner.upright_cross,
                    imup_cmd->inner.min_valid_cross,
                    imup_cmd->inner.max_valid_cross,
                    imup_cmd->inner.max_total_norm,
                    imup_cmd->inner.x_threshold,
                    imup_cmd->inner.z_threshold);
                valid_command++;
                break;
            case CMD_ID_LDDR:
                leddar_cmd = (LeddarCommand *)command_buffer;
                setLeddarParameters(leddar_cmd->inner.min_detection_distance,
                                    leddar_cmd->inner.max_detection_distance);
                break;
            default:
                invalid_command++;
                break;
        }
        command_length = 0;
        command_ready = false;
        sendCommandAcknowledge(last_command, valid_command, invalid_command);
    }
}
