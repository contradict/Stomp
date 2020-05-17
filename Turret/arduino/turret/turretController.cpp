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
#include "hammerController.h"
#include "flameThrowerController.h"
#include "selfRightController.h"

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
    m_lastUpdateTime = micros();
    setState(EInit);
}

void TurretController::Update()
{
    m_lastUpdateTime = micros();

    //  Pass update to our owned objects

    m_pTurretRotationController->Update();
    m_pHammerController->Update();
    m_pFlameThrowerController->Update();
    m_pSelfRightController->Update();

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

                if (micros() - m_stateStartTime > k_safeStateMinDt && isRadioConnected())
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
                if (hammerManualFire() && m_pHammerController->ReadyToFire())
                {
                    setState(EHammerTriggered);
                }
            }

            case EHammerTriggered:
            {
                if (m_pHammerController->ReadyToFire())
                {
                    setState(EActive);
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

int32_t TurretController::GetCurrentSpeed()
{
    m_pTurretRotationController->GetCurrentSpeed();
}

int32_t TurretController::GetDesiredManualTurretSpeed()
{
    return getDesiredManualTurretSpeed();
}

void TurretController::SetAutoAimParameters(int32_t p_proportionalConstant, int32_t p_derivativeConstant, int32_t p_steer_max, int32_t p_gyro_gain, uint32_t p_telemetry_interval)
{
    m_pTurretRotationController->SetAutoAimParameters(p_proportionalConstant, p_derivativeConstant, p_steer_max, p_gyro_gain, p_telemetry_interval);
}

void TurretController::SetAutoFireParameters(int16_t p_xtol, int16_t p_ytol, int16_t p_max_omegaz, uint32_t telemetry_interval)
{
    m_pHammerController->SetAutoFireParameters(p_xtol, p_ytol, p_max_omegaz, telemetry_interval);
}

void TurretController::SetParams(uint32_t p_watchDogTimerTriggerDt)
{
    m_params.WatchDogTimerTriggerDt = p_watchDogTimerTriggerDt;
    saveParams();
}

void TurretController::RestoreParams()
{
    m_pTurretRotationController->RestoreParams();
    m_pHammerController->RestoreParams();
    m_pFlameThrowerController->RestoreParams();
    m_pSelfRightController->RestoreParams();

    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct TurretController::Params));
}

void TurretController::SendTelem()
{
    m_pTurretRotationController->SendTelem();
    m_pHammerController->SendTelem();
    m_pFlameThrowerController->SendTelem();
    m_pSelfRightController->SendTelem();

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
    m_pHammerController->Init();
    m_pFlameThrowerController->Init();
    m_pSelfRightController->Init();
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
        case ESafe:
        {
            //  No action on leaving ESafe at this time
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
            m_pTurretRotationController = new TurretRotationController();
            m_pHammerController = new HammerController();
            m_pFlameThrowerController = new FlameThrowerController();
            m_pSelfRightController = new SelfRightController();

            initAllControllers();            
        }
        break;

        case ESafe:
        {
        }
        break;

        case EHammerTriggered:
        {
            m_pHammerController->Fire();
        }
        break;
    }
}

void TurretController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct TurretController::Params));
}