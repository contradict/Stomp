//
//  Turret Controller
//

#include "Arduino.h"
#include "pins.h"

#include "sbus.h"
#include "autoaim.h"
#include "DMASerial.h"

#include "turret_main.h"
#include "turretRotationController.h"
#include "hammerController.h"
#include "autofire.h"
#include "flameThrowerController.h"
#include "telemetryController.h"
#include "radioController.h"
#include "imuController.h"
#include "turretController.h"

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
    //  BB MJS: init the old turret_main
    turretInit();

    m_state = EInvalid;
    m_lastUpdateTime = micros();
    setState(EInit);
}

void TurretController::Update()
{
    //  BB MJS: update the old turret_main
    turretUpdate();

    m_lastUpdateTime = micros();

    //  Pass update to our owned objects

    m_pTurretRotationController->Update();
    m_pHammerController->Update();
    m_pFlameThrowerController->Update();
    m_pIMUController->Update();
    m_pAutoFireController->Update();

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
                    setState(EUnknown);
                }
            }
            break;

            case EUnknown:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (m_pIMUController->IsUpright() && m_pHammerController->ReadyToSwing())
                {
                    setState(ENominal);
                }
                else if (!m_pIMUController->IsUpright())
                {
                    setState(ENeedsSelfRight);
                }
            }
            break;

            case ENominal:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (m_pHammerController->ReadyToSwing() && hammerManualThrowAndRetract())
                {
                    setState(EHammerTriggerThrowRetract);
                }
                else if (hammerManualRetractOnly())
                {
                    setState(EHammerTriggerRetract);
                }
            }
            break;

            case EHammerTriggerThrowRetract:
            case EHammerTriggerRetract:
            {
                if (m_lastUpdateTime != m_stateStartTime)
                {
                    setState(EHammerActive);
                }
            }
            break;

            case EHammerActive:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (!isWeaponEnabled())
                {
                    setState(EUnknown);
                }
                else if (m_pHammerController->ReadyToSwing())
                {
                    if (m_pIMUController->IsUpright())
                    {
                        setState(ENominal);
                    }
                    else
                    {
                        setState(EUnknown);
                    }
                }
            }
            break;

            case ESelfRightTrigger:
            {
                setState(EHammerActive);
            }
            break;

            case ENeedsSelfRight:
            {
                if (m_pIMUController->IsUpright())
                {
                    setState(EUnknown);
                }
                if (m_lastUpdateTime - m_stateStartTime > k_selfRightTriggerDt)
                {
                    setState(ESelfRightTrigger);
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
}

bool TurretController::IsNominal()
{
    return m_state == ENominal;
}

Track* TurretController::GetCurrentTarget()
{
    return &g_trackedObject;
}

int16_t TurretController::GetTurretRotationSpeed()
{
    return m_pIMUController->GetOmegaZ();
}

int16_t TurretController::GetTurretAngle()
{
    return m_pTurretRotationController->GetTurretAngle();
}

int32_t TurretController::GetHammerSpeed()
{
    return m_pHammerController->GetHammerSpeed();
}

int16_t TurretController::GetHammerAngle()
{
    return m_pHammerController->GetHammerAngle();
}

int32_t TurretController::GetDesiredManualTurretSpeed()
{
    return getDesiredManualTurretSpeed();
}

void TurretController::FlamePulseStart()
{
    m_pFlameThrowerController->FlamePulseStart();
}

void TurretController::FlamePulseStop()
{
    m_pFlameThrowerController->FlamePulseStop();
}

void TurretController::SetAutoAimParameters(int32_t p_proportionalConstant, int32_t p_derivativeConstant, int32_t p_steer_max, int32_t p_gyro_gain)
{
    m_pTurretRotationController->SetAutoAimParameters(p_proportionalConstant, p_derivativeConstant, p_steer_max, p_gyro_gain);
}

void TurretController::SetAutoFireParameters(int16_t p_xtol, int16_t p_ytol, int16_t p_max_omegaz)
{
    m_pAutoFireController->SetParams(p_xtol, p_ytol, p_max_omegaz);
}

void TurretController::SetTurretRotationParameters(uint32_t p_manualControlOverideSpeed)
{
    m_pTurretRotationController->SetParams(p_manualControlOverideSpeed);
}

void TurretController::SetHammerParameters(uint32_t p_selfRightIntensity, uint32_t p_swingTelemetryFrequency)
{
    m_pHammerController->SetParams(p_selfRightIntensity, p_swingTelemetryFrequency);
}

void TurretController::SetIMUParameters(int8_t p_dlpf, int32_t p_imuPeriod, int32_t p_stationaryThreshold,
    int16_t p_uprightCross, int16_t p_minValidCross, int16_t p_maxValidCross,
    int16_t p_maxTotalNorm, int16_t p_xThreshold, int16_t p_zThreshold)
{
    m_pIMUController->SetParams(p_dlpf, p_imuPeriod, p_stationaryThreshold, 
        p_uprightCross, p_minValidCross, p_maxValidCross,
        p_maxTotalNorm, p_xThreshold, p_zThreshold);
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
    m_pIMUController->RestoreParams();
    m_pAutoFireController->RestoreParams();

    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct TurretController::Params));
}

void TurretController::SendTelem()
{
    turretSendTelem();      //  BB MSJ: This is the remaining stuff in turret_main.cpp 

    m_pTurretRotationController->SendTelem();
    m_pHammerController->SendTelem();
    m_pIMUController->SendTelem();
    m_pAutoFireController->SendTelem();

    Telem.SendTurretTelemetry(m_state);

}

void TurretController::SendLeddarTelem()
{
    turretSendLeddarTelem();
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void TurretController::init()
{
    m_pTurretRotationController = new TurretRotationController();
    m_pHammerController = new HammerController();
    m_pFlameThrowerController = new FlameThrowerController();
    m_pIMUController = new IMUController();
    m_pAutoFireController = new AutoFire();
}

void TurretController::initAllControllers()
{
    m_pTurretRotationController->Init();
    m_pHammerController->Init();
    m_pFlameThrowerController->Init();
    m_pIMUController->Init();
    m_pAutoFireController->Init();
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
            initAllControllers();
        }
        break;

        case ESafe:
        {
            //  Everybody should be in safe state, but for extra caution
            //  send an explict safe to all controllers

            m_pTurretRotationController->Safe();
            m_pHammerController->Safe();
            m_pFlameThrowerController->Safe();
        }
        break;

        case EHammerTriggerThrowRetract:
        {
            Telem.LogMessage("HammerTrigger");
            m_pHammerController->TriggerSwing();
        }
        break;

        case EHammerTriggerRetract:
        {
            m_pHammerController->TriggerRetract();
        }
        break;

        case ESelfRightTrigger:
        {
            m_pHammerController->TriggerSelfRightSwing();
        }
        break;

        default:
        break;
    }
}

void TurretController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct TurretController::Params));
}