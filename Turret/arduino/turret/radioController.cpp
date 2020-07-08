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
}

bool RadioController::IsManualTurretEnabled()
{
}

bool RadioController::IsAutoAimEnabled()
{
}

bool RadioController::IsAutoFireEnabled()
{
}

bool RadioController::IsSelfRightEnabled()
{
}

bool RadioController::IsFlameOnEnabled()
{
}

bool RadioController::IsFlamePulseEnabled()
{
}

bool RadioController::IsHammerSwingRequested()
{
}

bool RadioController::IsHammerRetractRequested()
{
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
    }
}

void RadioController::init()
{
}