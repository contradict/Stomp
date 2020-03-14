//
//  Flame Thrower Controller
//

#include "Arduino.h"
#include "pins.h"

#include "sbus.h"
#include "telem.h"
#include "autoaim.h"
#include "DMASerial.h"

#include "turretController.h"
#include "selfRightController.h"

//  ====================================================================
//
//  External references
//
//  ====================================================================

//  ====================================================================
//
//  File static variables
//
//  ====================================================================

static struct SelfRightController::Params EEMEM s_savedParams = 
{
    .tmp = 0,
};

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

void SelfRightController::Init()
{
    m_state = EInvalid;
    setState(EInit);
}

void SelfRightController::Update()
{
    uint32_t now = micros();

    //  Update our state

    while(true)
    {
        controllerState prevState = m_state;

        switch (m_state)
        {
            case EInit:
            {
                setState(ESafe);
            }
            break;

            case ESafe:
            {
                //  Stay in safe mode for a minimum of k_safeStateMinDt

                if (now - m_stateStartTime > k_safeStateMinDt && isRadioConnected())
                {
                    if (isSelfRightEnabled())
                    {
                        setState(EUnknownOrientation);
                    }
                    else
                    {
                        setState(EDisabled);
                    }
                }
            }
            break;

            case EDisabled:
            {
                if (!isRadioConnected())
                {
                    setState(ESafe);
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

    //  Now that the state is stable, take action based on stable state

}

void SelfRightController::SetParams()
{
    saveParams();
}

void SelfRightController::RestoreParams()
{
    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct SelfRightController::Params));
}

void SelfRightController::SendTelem()
{
    // sendSwingTelemetry(;
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void SelfRightController::setState(controllerState p_state)
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
    m_stateStartTime = micros();

    //  enter state transition

    switch (m_state)
    {
        case EInit:
        {
        }
        break;

        case ESafe:
        {
        }
        break;
    }
}


void SelfRightController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct SelfRightController::Params));
}