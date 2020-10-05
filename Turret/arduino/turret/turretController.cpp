//
//  Turret Controller
//

#include "Arduino.h"
#include "pins.h"

#include "sbus.h"
#include "autoAimController.h"
#include "DMASerial.h"

#include "turret_main.h"
#include "targetAcquisitionController.h"
#include "targetTrackingController.h"
#include "turretRotationController.h"
#include "hammerController.h"
#include "autoFireController.h"
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
    //  BB MJS: update the old turret_main, should remove at some point
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
                    setState(EUnknownOrientation);
                }
            }
            break;

            case EUnknownOrientation:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                if (Radio.IsSelfRightEnabled())
                {
                    if (m_pIMUController->IsUpright())
                    {
                        setState(ENominal);
                    }
                    else if (m_pIMUController->IsUpsideDown())
                    {
                        setState(ENeedsSelfRight);
                    }
                }
                else
                {
                    setState(ENominal);
                }
                
            }
            break;

            case ENominal:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (Radio.IsSelfRightEnabled())
                {
                    if (m_pIMUController->IsUpsideDown())
                    {
                        setState(ENeedsSelfRight);
                    }
                    else if (!m_pIMUController->IsUpright() && !m_pIMUController->IsUpsideDown())
                    {
                        setState(EUnknownOrientation);
                    }
                }
                else if (m_pHammerController->ReadyToSwing())
                {
                    setState(ENominalArmed);
                }
            }
            break;

            case ENominalArmed:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (!m_pHammerController->ReadyToSwing())
                {
                    setState(ENominal);
                }
                else if (Radio.IsSelfRightEnabled())
                {
                    if (m_pIMUController->IsUpsideDown())
                    {
                        setState(ENeedsSelfRight);
                    }
                    else if (!m_pIMUController->IsUpright() && !m_pIMUController->IsUpsideDown())
                    {
                        setState(EUnknownOrientation);
                    }
                }
                else if (Radio.IsHammerSwingRequested() || m_pAutoFireController->ShouldSwingAtTarget())
                {
                    setState(EHammerTriggerThrowRetract);
                }
                else if (Radio.IsHammerRetractRequested())
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
                else if (m_pHammerController->ReadyToSwing() || !isWeaponEnabled())
                {
                    setState(EUnknownOrientation);
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
                if (!Radio.IsSelfRightEnabled() || m_pIMUController->IsUpright())
                {
                    setState(ENominal);
                }
                else if (!m_pIMUController->IsUpsideDown())
                {
                    setState(EUnknownOrientation);
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
    return m_state == ENominal || m_state == ENominalArmed;
}

bool TurretController::IsNominalArmed()
{
    return m_state == ENominalArmed;
}

bool TurretController::IsSafeForFlameThrowers()
{
    return Radio.IsWeaponEnabled() && m_pHammerController->IsSafeForFlameThrowers();
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


int32_t TurretController::GetEstimatedSwingDuration()
{
    //  Given the 1 - 9 hammer throw intensities, return the 
    //  swing time in mircoseconds

    //  BB MJS: NEED TO GET REAL HAMMER SWING TIMES
    
    int32_t throwIntensity = Radio.GetHammerIntensity();
    return k_throwIntensityToSwingTime[throwIntensity];
}

void TurretController::FlamePulseStart()
{
    m_pFlameThrowerController->FlamePulseStart();
}

void TurretController::FlamePulseStop()
{
    m_pFlameThrowerController->FlamePulseStop();
}

void TurretController::SetAutoAimParameters(int32_t p_proportionalConstant, int32_t p_derivativeConstant, int32_t p_steerMax, int32_t p_gyroGain)
{
    m_pTurretRotationController->SetAutoAimParameters(p_proportionalConstant, p_derivativeConstant, p_steerMax, p_gyroGain);
}

void TurretController::SetAutoFireParameters(int16_t p_xtol, int16_t p_ytol, int16_t p_maxOmegaZ)
{
    m_pAutoFireController->SetParams(p_xtol, p_ytol, p_maxOmegaZ);
}

void TurretController::SetTurretRotationParameters(uint32_t p_manualControlOverideSpeed)
{
    m_pTurretRotationController->SetParams(p_manualControlOverideSpeed);
}

void TurretController::SetHammerParameters(uint32_t p_selfRightIntensity, 
        uint32_t p_swingTelemetryFrequency,
        uint16_t p_maxThrowAngle,
        uint16_t p_minRetractAngle,
        uint16_t p_emergencyBreakAngle,
        float p_throwSideBreakingForceTrigger,
        float p_breakStopAngle,
        uint32_t p_maxThrowUnderPressureDt,
        uint32_t p_maxThrowExpandDt,
        uint32_t p_maxRetractUnderPressureDt,
        uint32_t p_maxRetractExpandDt,
        uint32_t p_maxRetractBrakeDt,
        uint32_t p_maxRetractSettleDt,
        int32_t p_minBrakeExitVelocity)
{
    m_pHammerController->SetParams(p_selfRightIntensity, 
        p_swingTelemetryFrequency,
        p_maxThrowAngle,
        p_minRetractAngle,
        p_emergencyBreakAngle,
        p_throwSideBreakingForceTrigger,
        p_breakStopAngle,
        p_maxThrowUnderPressureDt,
        p_maxThrowExpandDt,
        p_maxRetractUnderPressureDt,
        p_maxRetractExpandDt,
        p_maxRetractBrakeDt,
        p_maxRetractSettleDt,
        p_minBrakeExitVelocity);
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
    //  Restore all the EEPROM parameters for controllers turret onws

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

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

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

void TurretController::init()
{
    m_pTurretRotationController = new TurretRotationController();
    m_pHammerController = new HammerController();
    m_pFlameThrowerController = new FlameThrowerController();
    m_pIMUController = new IMUController();
    m_pAutoFireController = new AutoFireController();
}

void TurretController::initAllControllers()
{
    m_pTurretRotationController->Init();
    m_pHammerController->Init();
    m_pFlameThrowerController->Init();
    m_pIMUController->Init();
    m_pAutoFireController->Init();
}

void TurretController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct TurretController::Params));
}
