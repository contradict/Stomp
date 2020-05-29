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
                if (g_outputChannel)
                {
                    setState(EUSBChannel);
                }
                else if (m_lastUpdateTime - m_stateStartTime > k_usbChannelTimeout)
                {
                    setState(EXBeeChannel);
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
        usbWrite(p_message);
    }
    else if (m_state == EXBeeChannel)
    {
        xbeeWrite(p_message);
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
        usbWrite(p_message);
    }
    else if (m_state == EXBeeChannel)
    {
        xbeeWrite(p_message);
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

void TelemetryController::usbWrite(const String& p_string)
{
    g_outputChannel.println(p_string);
}

void TelemetryController::xbeeWrite(const String& p_string)
{
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
        }
        break;

        case EChooseChannel:
        {
            //  First, try to init USB, then fall back to XBee if we can not

            initUSB();
        }
        break;

        case EUSBChannel:
        {
            if (m_invalidStateLog)
            {
                LogError("There was a call to Telem.Log before Serial Channel was ready");
            }
        }
        break;

        case EXBeeChannel:
        {
            g_outputChannel.end();
            initXBee();

            if (m_invalidStateLog)
            {
                LogError("There was a call to Telem.Log before Serial Channel was ready");
            }
        }
        break;
    }
}

void TelemetryController::initXBee()
{
    // Xbee Series 2 configuration notes:
    // Followed tutorial here: https://eewiki.net/display/Wireless/XBee+Wireless+Communication+Setup
    // Xbee SN 13A200 40BEFC5C is set to Coordinator AT, and DH/DL programmed to the SN of the Router AT
    // Xbee SN 13A200 40B9D1B1 is set to Router AT, and DH/DL programmed to the SN of the Coordinator AT
    // They're talking on PAN ID 2001 (A Space Odyssey)

/*
    g_outputChannel.begin(57600);
    pinMode(XBEE_CTS, INPUT);
    *digitalPinToPCMSK(XBEE_CTS) |= bit (digitalPinToPCMSKbit(XBEE_CTS));
    PCIFR  |= bit (digitalPinToPCICRbit(XBEE_CTS));
    PCICR  |= bit (digitalPinToPCICRbit(XBEE_CTS));
    g_outputChannel.set_cts_pin(XBEE_CTS);
*/
}

void TelemetryController::initUSB()
{
    g_outputChannel.begin(19200);
}

