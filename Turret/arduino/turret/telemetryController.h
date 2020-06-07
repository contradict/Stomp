#pragma once

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class TelemetryController
{
    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();

    size_t Write(const uint8_t *buffer, size_t size);
    bool Enqueue(const unsigned char *buffer, size_t size);

    void LogError(const String& p_message);
    void LogMessage(const String& p_message);
    
private:

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

    void xbeeSendMessage(const String&p_string);
    void usbSendMessage(const String& p_string);

    void setState(controllerState p_state);

    void initXBee();
    void initUSB();

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
};

extern TelemetryController Telem;