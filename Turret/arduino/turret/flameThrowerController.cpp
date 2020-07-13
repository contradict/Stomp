//
//  Flame Thrower Controller
//

#include "Arduino.h"
#include "pins.h"

#include "sbus.h"
#include "telemetryController.h"
#include "autoaim.h"
#include "DMASerial.h"

#include "turretController.h"
#include "radioController.h"
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
    m_lastUpdateTime = micros();

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

                if (m_lastUpdateTime - m_stateStartTime > k_safeStateMinDt && Radio.IsNominal())
                {
                    setState(EDisabled);
                }
            }
            break;

            case EDisabled:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (Radio.IsFlameOnEnabled() || Radio.IsFlamePulseEnabled())
                {
                    setState(EReadyToFire);
                }
            }
            break;

            case EReadyToFire:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (Radio.IsFlameOnEnabled())
                {
                    setState(EManualFlameOn);
                }
            }
            break;

            case EManualFlameOn:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (!Radio.IsFlameOnEnabled())
                {
                    setState(EDisabled);
                }
            }
            break;

            case EPulseFlameOn:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
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

    //  Now that the state is stable, take action based on stable state

}

void FlameThrowerController::Enable()
{
}

void FlameThrowerController::FlamePulseStart()
{
    setState(EPulseFlameOn);
}

void FlameThrowerController::FlamePulseStop()
{
    if (Radio.IsFlamePulseEnabled())
    {
        setState(EReadyToFire);
    }
    else
    {
        setState(EDisabled);
    }
}

void FlameThrowerController::Safe()
{
    setState(ESafe);
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

        case ESafe:
        {
            digitalWrite(PROPANE_DO, LOW);
            digitalWrite(IGNITER_DO, LOW);
        }
        break;

        case EDisabled:
        {
            digitalWrite(PROPANE_DO, LOW);
            digitalWrite(IGNITER_DO, LOW);
        }
        break;

        case EReadyToFire:
        {
            digitalWrite(PROPANE_DO, LOW);
            digitalWrite(IGNITER_DO, HIGH);
        }
        break;

        case EPulseFlameOn:
        {
            digitalWrite(PROPANE_DO, HIGH);
        }
        break;

        case EManualFlameOn:
        {
            digitalWrite(PROPANE_DO, HIGH);
        }
        break;

        default:
        break;
    }
}

void FlameThrowerController::init()
{
    //  Ensure everything is off
    
    digitalWrite(PROPANE_DO, LOW);
    digitalWrite(IGNITER_DO, LOW);
}