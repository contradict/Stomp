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

    void xbeeWrite(const String&p_string);
    void usbWrite(const String& p_string);

    void setState(controllerState p_state);

    void initXBee();
    void initUSB();

    //  ====================================================================
    //
    //  Private constants
    //
    //  ====================================================================
    
    const uint32_t k_usbChannelTimeout = 500000;

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