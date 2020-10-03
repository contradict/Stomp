//
//  Telemetry Controller
//

#include "Arduino.h"
#include "pins.h"

#include "DMASerial.h"
#include "leddar_io.h"

#include "turretController.h"
#include "targetTrackingController.h"
#include "telemetryController.h"

//  BB MJS: TODO
//
//  Provide methods for level of logging (errors, logs or both)
//  Ensure all telemetry is still working to XBee
//  Get LogMessage and ErrorMessage to work over XBee
//

//  Need to esnure that we can get the telemetry over XBee, reguardless of
//  the serial port selection logic.  Define FORCE_XBEE to true or false

#define FORCE_XBEE true
#define FORCE_USB false

#define CHECK_ENABLED(TLM_ID) if(!(m_params.enabledTelemetry & (0x1L << (TLM_ID)))) return false;
const uint16_t TLM_TERMINATOR=0x6666;


//  ====================================================================
//
//  File static variables
//
//  ====================================================================

static struct TelemetryController::Params EEMEM s_savedParams = 
{
    .telemetryInterval = 100000L,
    .leddarTelemetryInterval = 100000L,
    .enabledTelemetry = 
        (0x1L << (TelemetryController::TLM_ID_SYS))   |
        (0x1L << (TelemetryController::TLM_ID_SBS))   |
        (0x1L << (TelemetryController::TLM_ID_SNS))   |
        (0x1L << (TelemetryController::TLM_ID_TUR))   | 
        (0x1L << (TelemetryController::TLM_ID_LIDAR)) |
        (0x1L << (TelemetryController::TLM_ID_TROT))  |
        (0x1L << (TelemetryController::TLM_ID_AAIM))  |
        (0x1L << (TelemetryController::TLM_ID_ACK))
};

static DMASerial& s_TelemetrySerial = DSerial;

//  ====================================================================
//
//  Supporting Interrupt Service Routines
//
//  ====================================================================

ISR(PCINT2_vect)
{
   s_TelemetrySerial.cts_interrupt();
}

//  ====================================================================
//
//  Common template for all Telemetry Packets, each one is defined
//  right before the method that is going to send that type of packet
//
//  ====================================================================

template <uint8_t packet_id, typename packet_inner> struct TelemetryPacket
{
    uint8_t pkt_id;
    packet_inner inner;
    uint16_t terminator;
    TelemetryPacket() : pkt_id(packet_id), terminator(TLM_TERMINATOR) {};
} __attribute__((packed));

//  ====================================================================
//
//  Constructors
//
//  ====================================================================

//  ====================================================================
//
//  Public API methods
//
//  ====================================================================

void TelemetryController::Init()
{
    m_state = EInvalid;
    m_lastUpdateTime = micros();
    setState(EInit);

    //  Run update to see if we can get in a good state before the rest
    //  of the system initializes so we can log any of their messages

    Update();
}

void TelemetryController::Update()
{
    m_lastUpdateTime = micros();

    //  Update our state

    while(true)
    {
        controllerState prevState = m_state;

        switch (m_state)
        {
            case EInit:
            {
                setState(EChooseChannel);
            }
            break;

            case EChooseChannel:
            {
                if (FORCE_USB)
                {
                    setState(EUSBChannel);                  
                }
                else if (FORCE_XBEE || digitalRead(XBEE_CTS_DI))
                {
                    setState(EXBeeChannel);
                }
                else
                {
                    setState(EUSBChannel);
                }
            }
            break;

            default:
            break;
        }

        //  No more state changes, move on
        
        if (m_state == prevState)
        {
            break;
        }
    }

    sendTelem();
}

void TelemetryController::LogError(const String& p_message)
{
    if (m_state == EUSBChannel)
    {
        usbSendMessage(p_message);
    }
    else if (m_state == EXBeeChannel)
    {
        xbeeSendMessage(p_message);
    }
    else
    {
        m_invalidStateLog = true;
    }
}

void TelemetryController::LogMessage(const String& p_message)
{
    if (m_state == EUSBChannel)
    {
        usbSendMessage(p_message);
    }
    else if (m_state == EXBeeChannel)
    {
        xbeeSendMessage(p_message);
    }
    else
    {
        m_invalidStateLog = true;
    }
}

//
//  System Telemetry Packet
//

struct SystemTelemetryInner 
{
    uint8_t  weaponsEnabled:1;
    uint32_t loopSpeedMin;
    uint32_t loopSpeedAvg;
    uint32_t loopSpeedMax;
    uint32_t loopCount;
    uint16_t leddarOverrun;
    uint16_t leddarCRCError;
    uint16_t sbusOverrun;
    uint8_t lastCommand;
    uint16_t commandOverrun;
    uint16_t invalidCommand;
    uint16_t validCommand;
    uint32_t systemTime;
} __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_SYS, SystemTelemetryInner> SystemTelemetry;

bool TelemetryController::SendSystemTelem(
    uint32_t p_loopSpeedMin, uint32_t p_loopSpeedAvg, 
    uint32_t p_loopSpeedMax, uint32_t p_loopCount,
    uint16_t p_leddarOverrun, uint16_t p_leddarCRCError,
    uint16_t p_sbusOverrun, uint8_t p_lastCommand,
    uint16_t p_commandOverrun, uint16_t p_invalidCommand,
    uint16_t p_validCommand)
{
    CHECK_ENABLED(TLM_ID_SYS);

    SystemTelemetry tlm;

    tlm.inner.weaponsEnabled = g_enabled;
    tlm.inner.loopSpeedMin = p_loopSpeedMin;
    tlm.inner.loopSpeedAvg = p_loopSpeedAvg;
    tlm.inner.loopSpeedMax = p_loopSpeedMax;
    tlm.inner.loopCount = p_loopCount;
    tlm.inner.leddarOverrun = p_leddarOverrun;
    tlm.inner.leddarCRCError = p_leddarCRCError;
    tlm.inner.sbusOverrun = p_sbusOverrun;
    tlm.inner.lastCommand = p_lastCommand;
    tlm.inner.commandOverrun = p_commandOverrun;
    tlm.inner.invalidCommand = p_invalidCommand;
    tlm.inner.validCommand = p_validCommand;
    tlm.inner.systemTime = millis();

    return write((unsigned char *)&tlm, sizeof(tlm));
}

//
//  Sensor Telemetry Packet
//

struct SensorTelemetryInner 
{
    uint16_t angle;
    uint16_t throwPressure;
    uint16_t retractPressure;
}
 __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_SNS, SensorTelemetryInner> SensorTelemetry;

bool TelemetryController::SendSensorTelem(uint16_t p_angle, uint16_t p_throwPressure, uint16_t p_retractPressure)
{
    CHECK_ENABLED(TLM_ID_SNS);

    SensorTelemetry tlm;

    tlm.inner.angle = p_angle;
    tlm.inner.throwPressure = p_throwPressure;
    tlm.inner.retractPressure = p_retractPressure;

    return write((unsigned char *)&tlm, sizeof(tlm));
}

//
//  SBus Telemetry Packet
//

struct SBusTelemetryInner 
{
    uint16_t bitfield;
    int16_t hammerIntensity;
    int16_t hammerDistance;
    int16_t turretSpeed;
} __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_SBS, SBusTelemetryInner> SBusTelemetry;

bool TelemetryController::SendSbusTelem(uint16_t p_cmdBitfield, int16_t p_hammerIntensity, int16_t p_hammerDistance, int16_t p_turretSpeed) 
{
    CHECK_ENABLED(TLM_ID_SBS);

    SBusTelemetry tlm;

    tlm.inner.bitfield = p_cmdBitfield;
    tlm.inner.hammerIntensity = p_hammerIntensity;
    tlm.inner.hammerDistance = p_hammerDistance;
    tlm.inner.turretSpeed = p_turretSpeed;

    return write((unsigned char *)&tlm, sizeof(tlm));
}

//
//  SBus Telemetry Packet
//

struct LeddarTelemetryInner 
{
    uint16_t count;
    uint16_t range[LEDDAR_SEGMENTS];
    uint16_t amplitude[LEDDAR_SEGMENTS];
} __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_LIDAR, LeddarTelemetryInner> LeddarTelemetry;

static LeddarTelemetry leddar_tlm;

bool TelemetryController::SendLeddarTelem(const Detection (&p_detections)[LEDDAR_SEGMENTS], unsigned int count)
{
    CHECK_ENABLED(TLM_ID_LIDAR);

    leddar_tlm.inner.count = count;

    for (uint8_t i = 0; i < LEDDAR_SEGMENTS; i++)
    {
        leddar_tlm.inner.range[i] = p_detections[i].Distance;
        leddar_tlm.inner.amplitude[i] = p_detections[i].Amplitude;
    }

    return enqueue((unsigned char *)&leddar_tlm, sizeof(leddar_tlm));
}

//
//  Swing Telemetry Packet
//

struct SwingTelemetryInner 
{
    uint16_t dataPointCount;
    uint16_t sampleFrequency;
    uint32_t stopTimersTime;

    uint32_t swingStartTime;
    uint32_t swingExpandStartTime;

    uint32_t retractStartTime;
    uint32_t retractExpandStartTime; 
    uint32_t retractBreakStartTime;
    uint32_t retractStopTime;

    uint16_t swingStartAngle;
    uint16_t swingExpandStartAngle;
    uint16_t retractStartAngle;
    uint16_t retractExpandStartAngle;
    uint16_t retractBreakStartAngle;
    uint16_t retractStopAngle;
} __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_SWG, SwingTelemetryInner> SwingTelemetry;

bool TelemetryController::SendSwingTelem(
                    uint16_t p_datapointsCollected,
                    volatile uint16_t* p_angleData,
                    volatile uint8_t* p_throwPressureData,
                    volatile uint8_t* p_retractPressureData,
                    uint32_t p_sampleFrequency,
                    uint32_t p_stopTimersTime,

                    uint32_t p_swingStartTime, 
                    uint16_t p_swingStartAngle,
                    uint32_t p_swingExpandStartTime, 
                    uint16_t p_swingExpandStartAngle,
                    uint32_t p_retractStartTime, 
                    uint16_t p_retractStartAngle,
                    uint32_t p_retractExpandStartTime, 
                    uint16_t p_retractExpandStartAngle,
                    uint32_t p_retractBreakStartTime, 
                    uint16_t p_retractBreakStartAngle,
                    uint32_t p_retractStopTime, 
                    uint16_t p_retractStopAngle)
{
    //CHECK_ENABLED(TLM_ID_SWG);

    SwingTelemetry tlm;

    tlm.inner.sampleFrequency = p_sampleFrequency;
    tlm.inner.dataPointCount = p_datapointsCollected;
    tlm.inner.stopTimersTime = p_stopTimersTime;

    tlm.inner.swingStartTime = p_swingStartTime;
    tlm.inner.swingStartAngle = p_swingStartAngle;
    tlm.inner.swingExpandStartTime = p_swingExpandStartTime;
    tlm.inner.swingExpandStartAngle = p_swingExpandStartAngle;
    tlm.inner.retractStartTime = p_retractStartTime;
    tlm.inner.retractStartAngle = p_retractStartAngle;
    tlm.inner.retractExpandStartTime = p_retractExpandStartTime;
    tlm.inner.retractExpandStartAngle = p_retractExpandStartAngle;
    tlm.inner.retractBreakStartTime = p_retractBreakStartTime;
    tlm.inner.retractBreakStartAngle = p_retractBreakStartAngle;
    tlm.inner.retractStopTime = p_retractStopTime;
    tlm.inner.retractStopAngle = p_retractStopAngle;

    bool success = write((unsigned char *)&tlm, sizeof(tlm)-sizeof(TLM_TERMINATOR));

    if (success)
    {
        success &= enqueue((uint8_t *)p_angleData, sizeof(uint16_t) * 256);
        success &= enqueue((uint8_t *)p_throwPressureData, sizeof(int8_t) * 256);
        success &= enqueue((uint8_t *)p_retractPressureData, sizeof(int8_t) * 256);
    }

    success &= write((uint8_t *)&tlm.terminator, sizeof(tlm.terminator));
    return success;
}

//
//  IMU Telemetry Packet
//

struct IMUTelemetryInner 
{
    int16_t a[3];
    int16_t g[3];
    int16_t t;
} __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_IMU, IMUTelemetryInner> IMUTelemetry;

bool TelemetryController::SendIMUTelem(int16_t (&p_a)[3], int16_t (&p_g)[3], int16_t p_t)
{
    CHECK_ENABLED(TLM_ID_IMU);

    IMUTelemetry tlm;

    for(size_t i=0;i<3;i++) 
    {
        tlm.inner.a[i] = p_a[i];
        tlm.inner.g[i] = p_g[i];
    }

    tlm.inner.t = p_t;

    return write((unsigned char *)&tlm, sizeof(tlm));
}

//
//  Orientation Telemetry Packet
//

struct ORNTelemetryInner 
{
    uint8_t padding:3;
    uint8_t orientation:4;
    uint8_t stationary:1;
    int32_t sumAngularRate;
    int16_t totalNorm;
    int16_t crossNorm;
} __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_ORN, ORNTelemetryInner> ORNTelemetry;

bool TelemetryController::SendORNTelem(bool p_stationary, uint8_t p_orientation, int32_t p_sumAngularRate, int16_t p_totalNorm, int16_t p_crossNorm)
{
    CHECK_ENABLED(TLM_ID_ORN);

    ORNTelemetry tlm;

    tlm.inner.stationary = p_stationary;
    tlm.inner.orientation = p_orientation;
    tlm.inner.sumAngularRate = p_sumAngularRate;
    tlm.inner.totalNorm = p_totalNorm;
    tlm.inner.crossNorm = p_crossNorm;

    return write((unsigned char *)&tlm, sizeof(tlm));
}

//
//  Tracking Telemetry Packet
//

struct TrackingTelemetryInner 
{
    int8_t state;
    int16_t detectionX;
    int16_t detectionY;
    int32_t detectionAngle;
    int32_t detectionRadius;
    int16_t filteredX;
    int16_t filteredVx;
    int16_t filteredY;
    int16_t filteredVy;
} __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_TRK, TrackingTelemetryInner> TRKTelemetry;

bool TelemetryController::SendTrackingTelemetry(int8_t p_state,
    int16_t p_detectionX,
    int16_t p_detectionY,
    int32_t p_detectionAngle,
    int32_t p_detectionRadius,
    int32_t p_filteredX,
    int32_t p_filteredVx,
    int32_t p_filteredY,
    int32_t p_filteredVy) 
{
    CHECK_ENABLED(TLM_ID_TRK);

    TRKTelemetry tlm;

    tlm.inner.state = p_state;
    tlm.inner.detectionX = p_detectionX;
    tlm.inner.detectionY = p_detectionY;
    tlm.inner.detectionAngle = p_detectionAngle;
    tlm.inner.detectionRadius = p_detectionRadius;
    tlm.inner.filteredX = (int16_t)constrain(p_filteredX, -32768L, 32767L);
    tlm.inner.filteredVx = (int16_t)constrain(p_filteredVx, -32768L, 32767L);
    tlm.inner.filteredY = (int16_t)constrain(p_filteredY, -32768L, 32767L);
    tlm.inner.filteredVy = (int16_t)constrain(p_filteredVy, -32768L, 32767L);

    return write((unsigned char *)&tlm, sizeof(tlm));
}

//
//  AutoFireController Telemetry Packet
//

struct AutoFireTelemetryInner 
{
    int8_t state;
    int32_t swing;
    int16_t x;
    int16_t y;
} __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_AF, AutoFireTelemetryInner> AFTelemetry;

bool TelemetryController::SendAutoFireTelemetry(int32_t p_state, int32_t p_swing, int32_t p_x, int32_t p_y) 
{
    CHECK_ENABLED(TLM_ID_AF);

    AFTelemetry tlm;

    tlm.inner.state = p_state;
    tlm.inner.swing = p_swing;
    tlm.inner.x = (int16_t)constrain(p_x, -32768L, 32767L);
    tlm.inner.y = (int16_t)constrain(p_y, -32768L, 32767L);

    return write((unsigned char *)&tlm, sizeof(tlm));
}

//
//  AutoAimController Telemetry Packet
//

struct AutoAimTelemetryInner 
{
    int32_t state;
    int32_t targetAngularVelocity;
    int32_t error;
    int32_t errorIntegral;
    int32_t errorDerivitive;
} __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_AAIM, AutoAimTelemetryInner> AAIMTelemetry;

bool TelemetryController::SendAutoAimTelemetry(int32_t p_state, int32_t p_targetAngularVelocity, int32_t p_error, int32_t p_errorIntegral,  int32_t p_errorDerivitive) 
{
    CHECK_ENABLED(TLM_ID_AAIM);

    AAIMTelemetry tlm;

    tlm.inner.state = p_state;
    tlm.inner.targetAngularVelocity = p_targetAngularVelocity;
    tlm.inner.error = p_error;
    tlm.inner.errorIntegral = p_errorIntegral;
    tlm.inner.errorDerivitive = p_errorDerivitive;

    return write((unsigned char *)&tlm, sizeof(tlm));
}

//
//  Turret Telemetry Packet
//

struct TurretTelemetryInner 
{
    int16_t state;
} __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_TUR, TurretTelemetryInner> TURTelemetry;

bool TelemetryController::SendTurretTelemetry(int16_t p_state) 
{
    CHECK_ENABLED(TLM_ID_TUR);

    TURTelemetry tlm;

    tlm.inner.state = p_state;

    return write((unsigned char *)&tlm, sizeof(tlm));
}

//
//  Turret Rotation Telemetry Packet
//

struct TurretRotationTelemetryInner {
    int16_t state;
    int16_t currentSpeed;
} __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_TROT, TurretRotationTelemetryInner> TurretRotationTelemetry;

bool TelemetryController::SendTurretRotationTelemetry(int16_t p_state, int16_t p_currentSpeed) 
{
    CHECK_ENABLED(TLM_ID_TROT);

    TurretRotationTelemetry tlm;

    tlm.inner.state = p_state;
    tlm.inner.currentSpeed = p_currentSpeed;

    return write((unsigned char *)&tlm, sizeof(tlm));
}

bool TelemetryController::SendObjectsTelemetry(uint8_t p_numObjects, int8_t p_bestTarget, const Target (&p_objects)[8])
{
    bool sentCalculatedObjects = sendObjectsCalculatedTelemetry(p_numObjects, p_bestTarget, p_objects);
    bool sentMeasuredObjects = sendObjectsMeasuredTelemetry(p_numObjects, p_bestTarget, p_objects);

    return sentCalculatedObjects && sentMeasuredObjects;
}

struct ObjectsCalcuatedInner 
{
   uint8_t numObjects;
   int8_t bestTarget;
   int16_t objectDistance[8];
   int16_t objectAngle[8];
   int16_t objectX[8];
   int16_t objectY[8];
} __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_OBJC, ObjectsCalcuatedInner> ObjectsCalculatedTelemetry;

bool TelemetryController::sendObjectsCalculatedTelemetry(uint8_t p_numObjects, int8_t p_bestTarget, const Target (&p_objects)[8])
{
    CHECK_ENABLED(TLM_ID_OBJC);
    
    ObjectsCalculatedTelemetry tlm;
    
    tlm.inner.numObjects = p_numObjects;
    tlm.inner.bestTarget = p_bestTarget;
    
    int i=0;

    for(; i < p_numObjects; i++)
    {
        tlm.inner.objectDistance[i] = p_objects[i].GetDistance();
        tlm.inner.objectAngle[i] = p_objects[i].GetAngle();
        tlm.inner.objectX[i] = p_objects[i].GetXCoord();
        tlm.inner.objectY[i] = p_objects[i].GetYCoord();
    }

    for(; i < 8; i++)
    {
        tlm.inner.objectDistance[i] = -1;
        tlm.inner.objectAngle[i] = -1;
        tlm.inner.objectX[i] = -1;
        tlm.inner.objectY[i] = -1;
    }
    
    return write((unsigned char *)&tlm, sizeof(tlm));
}

struct ObjectsMeasuredInner 
{
    uint8_t numObjects;
    int8_t bestTarget;
    uint8_t leftEdge[8];
    uint8_t rightEdge[8];
    int16_t objectSumDistance[8];
} __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_OBJM, ObjectsMeasuredInner> ObjectsMeasuredTelemetry;

bool TelemetryController::sendObjectsMeasuredTelemetry(uint8_t p_numObjects, int8_t p_bestTarget, const Target (&p_objects)[8])
{
    CHECK_ENABLED(TLM_ID_OBJM);

    ObjectsMeasuredTelemetry tlm;

    tlm.inner.numObjects = p_numObjects;
    tlm.inner.bestTarget = p_bestTarget;

    int i=0;

    for(; i < p_numObjects; i++)
    {
        tlm.inner.leftEdge[i] = p_objects[i].LeftEdge;
        tlm.inner.rightEdge[i] = p_objects[i].RightEdge;
        tlm.inner.objectSumDistance[i] = p_objects[i].SumDistance;
    }

    for(; i < 8; i++)
    {
        tlm.inner.leftEdge[i] = -1;
        tlm.inner.rightEdge[i] = -1;
        tlm.inner.objectSumDistance[i] = -1;
    }
    return write((unsigned char *)&tlm, sizeof(tlm));
}

struct CommandAcknolwedgeInner 
{
    uint8_t cmdid;
    uint16_t valid;
    uint16_t invalid;
} __attribute__((packed));

typedef TelemetryPacket<TelemetryController::TLM_ID_ACK, CommandAcknolwedgeInner> ACKTelemetry;

bool TelemetryController::SendCommandAcknowledge(uint8_t p_command, uint16_t p_valid, uint16_t p_invalid) 
{
    CHECK_ENABLED(TLM_ID_ACK);
 
    ACKTelemetry tlm;
 
    tlm.inner.cmdid = p_command;
    tlm.inner.valid = p_valid;
    tlm.inner.invalid = p_invalid;
 
    return write((unsigned char *)&tlm, sizeof(tlm));
}

//
//  Telemetry Controller persistant parameters
//

void TelemetryController::SetParams(int32_t p_telemetryInterval, int32_t p_leddarTelemetryInterval, int32_t p_enabledTelemetry)
{
    m_params.telemetryInterval = p_telemetryInterval;
    m_params.leddarTelemetryInterval = p_leddarTelemetryInterval;
    m_params.enabledTelemetry = p_enabledTelemetry;

    saveParams();
}

void TelemetryController::RestoreParams()
{
    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct TelemetryController::Params));
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void TelemetryController::sendTelem()
{
    //  Check Leddar telemetry update first, then the general telemetry next

    if (m_lastUpdateTime - m_lastLeddarTelemTime > m_params.leddarTelemetryInterval)
    {
        TargetTracking.SendLeddarTelem();

        m_lastLeddarTelemTime = m_lastUpdateTime;

    }

    if (m_lastUpdateTime - m_lastTelemTime > m_params.telemetryInterval)
    {
        TargetTracking.SendTelem();
        Turret.SendTelem();

        m_lastTelemTime = m_lastUpdateTime;
    }
}

size_t TelemetryController::write(const uint8_t *buffer, size_t size)
{
    //  Don't send binary data in USBChannel state
    
    if (m_state != EXBeeChannel)
    {
        return 0;
    }

    return s_TelemetrySerial.write(buffer, size);
}

bool TelemetryController::enqueue(const unsigned char *buffer, size_t size)
{
    //  Don't send binary data in USBChannel state
        
    if (m_state != EXBeeChannel)
    {
        return false;
    }

    return s_TelemetrySerial.enqueue(buffer, size, NULL, NULL);
}

bool TelemetryController::usbSendMessage(const String& p_string)
{
    s_TelemetrySerial.println(p_string);
    return true;
}

bool TelemetryController::xbeeSendMessage(const String& p_string)
{
    CHECK_ENABLED(TLM_ID_DBGM);

    const char *c_str = p_string.c_str();

    unsigned char pkt[1+k_maxDebugMessageLength+2];

    pkt[0] = TLM_ID_DBGM;
    size_t copied = 0;
    size_t pos=1;

    while(copied < k_maxDebugMessageLength && c_str[copied]) 
    {
        pkt[pos++] = c_str[copied++];
    }

    *((uint16_t *)(pkt+pos)) = TLM_TERMINATOR;
    size_t sendlen = 1+copied+sizeof(TLM_TERMINATOR);
    
    return (s_TelemetrySerial.write(pkt, sendlen) == sendlen);
}

void TelemetryController::setState(controllerState p_state)
{
    if (m_state == p_state)
    {
        return;
    }

    //  exit state transition

    switch (m_state)
    {
        case EInit:
        {
        }
        break;

        default:
        break;
    }

    m_state = p_state;
    m_stateStartTime = m_lastUpdateTime;

    //  enter state transition

    switch (m_state)
    {
        case EInit:
        {
            init();
        }
        break;

        case EUSBChannel:
        {
            initUSB();

            if (m_invalidStateLog)
            {
                LogError("There was a call to Telem.Log before Serial Channel was ready");
            }
        }
        break;

        case EXBeeChannel:
        {
            initXBee();

            if (m_invalidStateLog)
            {
                LogError("There was a call to Telem.Log before Serial Channel was ready");
            }
        }
        break;

        default:
        break;
    }
}

void TelemetryController::init()
{
    m_lastTelemTime = micros();
    m_lastLeddarTelemTime = micros();

    m_invalidStateLog = false;
            
    pinMode(XBEE_CTS_DI, INPUT);
}

void TelemetryController::initUSB()
{
    s_TelemetrySerial.begin(57600);
}

void TelemetryController::initXBee()
{
    // Xbee Series 2 configuration notes:
    // Followed tutorial here: https://eewiki.net/display/Wireless/XBee+Wireless+Communication+Setup
    // Xbee SN 13A200 40BEFC5C is set to Coordinator AT, and DH/DL programmed to the SN of the Router AT
    // Xbee SN 13A200 40B9D1B1 is set to Router AT, and DH/DL programmed to the SN of the Coordinator AT
    // They're talking on PAN ID 2001 (A Space Odyssey)

    s_TelemetrySerial.begin(57600);
    *digitalPinToPCMSK(XBEE_CTS_DI) |= bit (digitalPinToPCMSKbit(XBEE_CTS_DI));

    PCIFR  |= bit (digitalPinToPCICRbit(XBEE_CTS_DI));
    PCICR  |= bit (digitalPinToPCICRbit(XBEE_CTS_DI));

    s_TelemetrySerial.set_cts_pin(XBEE_CTS_DI);
}

void TelemetryController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct TelemetryController::Params));
}
