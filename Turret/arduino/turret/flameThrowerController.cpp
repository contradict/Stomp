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
#include "flameThrowerController.h"

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

static struct FlameThrowerController::Params EEMEM s_savedParams = 
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

void FlameThrowerController::Init()
{
    m_state = EInvalid;
    m_lastUpdateTime = micros();
    setState(EInit);
}

void FlameThrowerController::Update()
{
    m_lastUpdateTime= micros();

    //  Pass update to our owned objects

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

                if (m_lastUpdateTime - m_stateStartTime > k_safeStateMinDt && isRadioConnected())
                {
                    if (isWeaponEnabled())
                    {
                        setState(EReadyToFire);
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

void FlameThrowerController::Safe()
{
    setState(ESafe);
}

void FlameThrowerController::SetParams()
{
    saveParams();
}

void FlameThrowerController::RestoreParams()
{
    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct FlameThrowerController::Params));
}

void FlameThrowerController::SendTelem()
{
    // sendSwingTelemetry(;
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void FlameThrowerController::setState(controllerState p_state)
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
        }
        break;

        case ESafe:
        {
        }
        break;
    }
}

void FlameThrowerController::initAllControllers()
{
}

void FlameThrowerController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct FlameThrowerController::Params));
}