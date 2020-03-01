//
//  Turret Controller
//

#include "Arduino.h"
#include "pins.h"

#include "sbus.h"
#include "telem.h"
#include "autoaim.h"
#include "DMASerial.h"

#include "turretController.h"
#include "turretRotationController.h"

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

static TurretRotationController s_turretRotationController;

static struct TurretController::Params EEMEM s_savedParams = 
{
    .WatchDogTimerTriggerDt = 4000000,
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

void TurretController::Init()
{
    m_state = EInvalid;
    setState(EInit);
}

void TurretController::Update()
{
    uint32_t now = micros();

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
                    setState(EActive);
                }
            }
            break;

            case EActive:
            {
                if (!isRadioConnected())
                {
                    setState(ESafe);
                }
            }
            break;
        }

        if (m_state == prevState)
        {
            break;
        }
    }

    //  Now that the state is stable, take action based on stable state

    s_turretRotationController.Update();
}

int32_t TurretController::GetDesiredAutoAimSpeed()
{
    return desiredAutoAimTurretSpeed();
}

int32_t TurretController::GetDesiredSBusSpeed()
{
    return desiredSBusTurretSpeed();
}

void TurretController::SetParams(uint32_t p_watchDogTimerTriggerDt)
{
    m_params.WatchDogTimerTriggerDt = p_watchDogTimerTriggerDt;
    saveParams();
}

void TurretController::RestoreParams()
{
    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct TurretController::Params));
}

void TurretController::SendTelem()
{
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void TurretController::initAllControllers()
{
    s_turretRotationController.Init();
}

void TurretController::setState(controllerState p_state)
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
            initAllControllers();
        }
        break;
    }

    m_state = p_state;
    m_stateStartTime = micros();

    //  enter state transition

    switch (m_state)
    {
        case ESafe:
        {
        }
        break;
    }
}

void TurretController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct TurretController::Params));
}