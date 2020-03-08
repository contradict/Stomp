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

extern Track g_trackedObject;

//  ====================================================================
//
//  File static variables
//
//  ====================================================================

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

    //  Pass update to our owned objects

    m_pTurretRotationController->Update();

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

        //  No more state changes, move on
        
        if (m_state == prevState)
        {
            break;
        }
    }
}

Track* TurretController::GetCurrentTarget()
{
    return &g_trackedObject;
}

int32_t TurretController::GetDesiredManualTurretSpeed()
{
    return getDesiredManualTurretSpeed();
}

void TurretController::SetParams(uint32_t p_watchDogTimerTriggerDt)
{
    m_params.WatchDogTimerTriggerDt = p_watchDogTimerTriggerDt;
    saveParams();
}

void TurretController::RestoreParams()
{
    m_pTurretRotationController->RestoreParams();

    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct TurretController::Params));
}

void TurretController::SendTelem()
{
    m_pTurretRotationController->SendTelem();
    sendTurretTelemetry(m_state);
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void TurretController::initAllControllers()
{
    m_pTurretRotationController->Init();
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
            m_pTurretRotationController = new TurretRotationController();
            initAllControllers();            
        }
        break;

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