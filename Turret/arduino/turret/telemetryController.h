#pragma once

#include "leddar_io.h"
#include "target.h"

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class TelemetryController
{
    //  ====================================================================
    //
    //  Public Enums
    //
    //  ====================================================================

public: 

    enum TelemetryPacketId 
    {
        // =1,
        // =2,
        // =3,
        TLM_ID_TRK=4,
        TLM_ID_AF=5,
        TLM_ID_ACK=6,
        TLM_ID_AAIM=7,
        TLM_ID_TUR=8,
        TLM_ID_TROT=9,
        TLM_ID_SNS=10,
        TLM_ID_SYS=11,
        TLM_ID_SBS=12,
        TLM_ID_DBGM=13,
        TLM_ID_SWG=14,
        TLM_ID_LIDAR=15,
        // =16,
        TLM_ID_IMU=17,
        // =18,
        TLM_ID_ORN=19,
        // =20,
        TLM_ID_OBJM=21,
        TLM_ID_OBJC=22,
    };
    
    //  ====================================================================
    //
    //  Internal structure decleration
    //
    //  ====================================================================

public:

    struct Params 
    {
        uint32_t telemetryInterval;
        uint32_t leddarTelemetryInterval;
        uint32_t enabledTelemetry;
    } __attribute__((packed));

    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();

    void LogError(const String& p_message);
    void LogMessage(const String& p_message);

    //  All the supported send telemetry commands

    bool SendSystemTelem(uint32_t p_loopSpeedMin, uint32_t p_loopSpeedAvg, 
        uint32_t p_loopSpeedMax, uint32_t p_loopCount,
        uint16_t p_leddarOverrun, uint16_t p_leddarCRCError,
        uint16_t p_sbusOverrun, uint8_t p_lastCommand,
        uint16_t p_commandOverrun, uint16_t p_invalidCommand,
        uint16_t p_validCommand);

    bool SendSensorTelem(uint16_t p_angle, uint16_t p_throwPressure, uint16_t p_retractPressure);
    bool SendSbusTelem(uint16_t p_cmdBitfield, int16_t p_hammerIntensity, int16_t p_hammerDistance, int16_t p_turretSpeed);
    bool SendLeddarTelem(const Detection (&p_detections)[LEDDAR_SEGMENTS], unsigned int count);

    bool SendSwingTelem(
        uint16_t p_datapointsCollected,
        volatile uint16_t* p_angleData,
        volatile uint8_t* p_throwPressureData,
        volatile uint8_t* p_retractPressureData,
        uint16_t p_dataCollectFrequency,
        uint32_t p_swingStartTime,
        uint16_t p_swingStartAngle,
        uint32_t p_swingStopTime,
        uint16_t p_swingStopAngle,
        uint32_t p_retractStartTime,
        uint16_t p_retractStartAngle,
        uint32_t p_retractStopTime,
        uint16_t p_retractStopAngle);

    bool SendIMUTelem(int16_t (&p_a)[3], int16_t (&p_g)[3], int16_t p_t);
    bool SendORNTelem(bool p_stationary, uint8_t p_orientation, int32_t p_sumAngularRate, int16_t p_totalNorm, int16_t p_crossNorm);

    bool SendTrackingTelemetry(int8_t p_state,
        int16_t p_detectionX,
        int16_t p_detectionY,
        int32_t p_detectionAngle,
        int32_t p_detectionRadius,
        int32_t p_filteredX,
        int32_t p_filteredVx,
        int32_t p_filteredY,
        int32_t p_filteredVy);

    bool SendAutoFireTelemetry(int32_t p_state, int32_t p_swing, int32_t p_x, int32_t p_y);
    bool SendAutoAimTelemetry(int32_t p_state, int32_t p_targetAngularVelocity, int32_t p_error, int32_t p_errorIntegral,  int32_t p_errorDerivitive);
    bool SendTurretTelemetry(int16_t p_state);
    bool SendTurretRotationTelemetry(int16_t p_state, int16_t p_currentSpeed) ;

    bool SendObjectsTelemetry(uint8_t p_numObjects, const Target (&p_objects)[8]);

    bool SendCommandAcknowledge(uint8_t p_command, uint16_t p_valid, uint16_t p_invalid) ;

    //  deal with persistant parameters

    void SetParams(int32_t p_telemetryInterval, int32_t p_leddarTelemetryInterval, int32_t p_enabledTelemetry);
    void RestoreParams();
    
private:

    enum controllerState 
    {
        EInit,

        EChooseChannel,
        EXBeeChannel,
        EUSBChannel,

        EInvalid = -1
    };

    //  ====================================================================
    //
    //  Private methods
    //
    //  ====================================================================

    void sendTelem();
    bool sendObjectsCalculatedTelemetry(uint8_t p_numObjects, const Target (&p_objects)[8]);
    bool sendObjectsMeasuredTelemetry(uint8_t p_numObjects, const Target (&p_objects)[8]);


    size_t write(const uint8_t *buffer, size_t size);
    bool enqueue(const unsigned char *buffer, size_t size);

    bool xbeeSendMessage(const String&p_string);
    bool usbSendMessage(const String& p_string);

    void setState(controllerState p_state);

    void init();
    void initXBee();
    void initUSB();

    void saveParams();

    //  ====================================================================
    //
    //  Private constants
    //
    //  ====================================================================
    
    const uint32_t k_xbeeChannelTimeout = 500000;
    const size_t k_maxDebugMessageLength = 128;

    //  ====================================================================
    //
    //  Private members
    //
    //  ====================================================================
    
    controllerState m_state;
    uint32_t m_lastUpdateTime;
    uint32_t m_stateStartTime;

    bool m_invalidStateLog;

    uint32_t m_lastTelemTime;
    uint32_t m_lastLeddarTelemTime;

    Params m_params;
};

extern TelemetryController Telem;