//
//  Telemetry Controller
//

#include "Arduino.h"
#include "pins.h"

#include "DMASerial.h"

#include "telemetryController.h"

//  BB MJS: TODO
//
//  Provide methods for level of logging (errors, logs or both)
//  Ensure all telemetry is still working to XBee
//  Get LogMessage and ErrorMessage to work over XBee
//

//  Need to esnure that we can get the telemetry over XBee, reguardless of
//  the serial port selection logic.  Define FORCE_XBEE to true or false

#define FORCE_XBEE false
#define FORCE_USB true

//  ====================================================================
//
//  File static variables
//
//  ====================================================================

static DMASerial& g_outputChannel = DSerial;

ISR(PCINT2_vect)
{
   g_outputChannel.cts_interrupt();
}

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
                else if (FORCE_XBEE || digitalRead(XBEE_CTS))
                {
                    setState(EXBeeChannel);
                }
                else
                {
                    setState(EUSBChannel);
                }
            }
            break;
        }

        //  No more state changes, move on
        
        if (m_state == prevState)
        {
            break;
        }
    }
}

size_t TelemetryController::Write(const uint8_t *buffer, size_t size)
{
    //  Don't send binary data in USBChannel state
    
    if (m_state != EXBeeChannel)
    {
        return 0;
    }

    return g_outputChannel.write(buffer, size);
}

bool TelemetryController::Enqueue(const unsigned char *buffer, size_t size)
{
    //  Don't send binary data in USBChannel state
        
    if (m_state != EXBeeChannel)
    {
        return false;
    }

    return g_outputChannel.enqueue(buffer, size, NULL, NULL);
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

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void TelemetryController::usbSendMessage(const String& p_string)
{
    g_outputChannel.println(p_string);
}

const uint16_t TLM_TERMINATOR=0x6666;

void TelemetryController::xbeeSendMessage(const String& p_string)
{
    const char *c_str = p_string.c_str();
    // BB MJS: CHECK_ENABLED(TLM_ID_DBGM);

    unsigned char pkt[1+k_maxDebugMessageLength+2]={0};
    pkt[0] = TLM_ID_DBGM;
    size_t copied = 0;
    size_t pos=1;

    while(copied < k_maxDebugMessageLength && c_str[copied]) 
    {
        pkt[pos++] = c_str[copied++];
    }

    *((uint16_t *)(pkt+pos)) = TLM_TERMINATOR;
    size_t sendlen = 1+copied+sizeof(TLM_TERMINATOR);
    
    // g_outputChannel.write(pkt, sendlen);
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
    }

    m_state = p_state;
    m_stateStartTime = m_lastUpdateTime;

    //  enter state transition

    switch (m_state)
    {
        case EInit:
        {
            m_invalidStateLog = false;
            
            pinMode(XBEE_CTS, INPUT);
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
    }
}

void TelemetryController::initUSB()
{
    g_outputChannel.begin(57600);
}

void TelemetryController::initXBee()
{
    // Xbee Series 2 configuration notes:
    // Followed tutorial here: https://eewiki.net/display/Wireless/XBee+Wireless+Communication+Setup
    // Xbee SN 13A200 40BEFC5C is set to Coordinator AT, and DH/DL programmed to the SN of the Router AT
    // Xbee SN 13A200 40B9D1B1 is set to Router AT, and DH/DL programmed to the SN of the Coordinator AT
    // They're talking on PAN ID 2001 (A Space Odyssey)

    g_outputChannel.begin(57600);
    *digitalPinToPCMSK(XBEE_CTS) |= bit (digitalPinToPCMSKbit(XBEE_CTS));
    PCIFR  |= bit (digitalPinToPCICRbit(XBEE_CTS));
    PCICR  |= bit (digitalPinToPCICRbit(XBEE_CTS));

    g_outputChannel.set_cts_pin(XBEE_CTS);
}