//
//  Radio Controller
//

#include "Arduino.h"

#include "pins.h"
#include "sbus.h"

#include "radioController.h"

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

void RadioController::Init()
{
    m_state = EInvalid;
    m_lastUpdateTime = micros();

    setState(EInit);
}

void RadioController::Update()
{
    m_lastUpdateTime = micros();

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

bool RadioController::IsNominal()
{
    return isRadioConnected();
}

bool RadioController::IsWeaponEnabled()
{
    return isWeaponEnabled();
}

bool RadioController::IsManualTurretEnabled()
{
    return isManualTurretEnabled();
}

bool RadioController::IsAutoAimEnabled()
{
    return isAutoAimEnabled();
}

bool RadioController::IsAutoFireEnabled()
{
    return isAutoFireEnabled();
}

bool RadioController::IsSelfRightEnabled()
{
    return isSelfRightEnabled();
}

bool RadioController::IsFlameOnEnabled()
{
    // BB MJS: Implement

    return false;
}

bool RadioController::IsFlamePulseEnabled()
{
    // BB MJS: Implement

    return false;
}

bool RadioController::IsHammerSwingRequested()
{
    return hammerManualThrowAndRetract();
}

bool RadioController::IsHammerRetractRequested()
{
    return hammerManualRetractOnly();
}

int32_t RadioController::GetDesiredManualTurretSpeed()
{
    return getDesiredManualTurretSpeed();
}

void RadioController::SendTelem()
{
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void RadioController::setState(controllerState p_state)
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
        }
        break;

        default:
        break;
    }
}

void RadioController::init()
{
}