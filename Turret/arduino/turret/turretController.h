#pragma once

#include "track.h"

//  ====================================================================
//
//  Forward declerations
//
//  ====================================================================

class TurretRotationController;
class HammerController;
class FlameThrowerController;
class IMUController;
class AutoFire;

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class TurretController
{

    //  ====================================================================
    //
    //  Internal structure decleration
    //
    //  ====================================================================

public:

    struct Params
    {
        int32_t WatchDogTimerTriggerDt;
    };

    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();

    bool IsNominal();
    
    Track* GetCurrentTarget();

    int16_t GetTurretRotationSpeed();
    int16_t GetTurretAngle();
    
    int32_t GetHammerSpeed();
    int16_t GetHammerAngle();

    int32_t GetDesiredManualTurretSpeed();

    void FlamePulseStart();
    void FlamePulseStop();

    void SetAutoAimParameters(int32_t p_proportionalConstant, int32_t p_derivativeConstant, int32_t p_steer_max, int32_t p_gyro_gain);
    void SetAutoFireParameters(int16_t p_xtol, int16_t p_ytol, int16_t p_max_omegaz);
    void SetHammerParameters(uint32_t p_selfRightIntensity, uint32_t p_swingTelemetryFrequency);
    void SetTurretRotationParameters(uint32_t p_manualControlOverideSpeed);
    void SetIMUParameters(int8_t p_dlpf, int32_t p_imuPeriod, int32_t p_stationaryThreshold,
        int16_t p_uprightCross, int16_t p_minValidCross, int16_t p_maxValidCross,
        int16_t p_maxTotalNorm, int16_t p_xThreshold, int16_t p_zThreshold);

    void SetParams(uint32_t p_watchDogTimerTriggerDt);
    void RestoreParams();

    void SendTelem();
    void SendLeddarTelem();

private:

    enum controllerState 
    {
        EInit,
        ESafe,

        ENominal,

        EHammerTriggerThrowRetract,
        EHammerTriggerRetract,
        EHammerActive,

        ENeedsSelfRight,
        ESelfRightTrigger,

        EUnknown,
        EDegraded,

        EInvalid = -1
    };

    //  ====================================================================
    //
    //  Private methods
    //
    //  ====================================================================

    void setState(controllerState p_state);

    void init();
    void initAllControllers();

    void saveParams();

    //  ====================================================================
    //
    //  Private constants
    //
    //  ====================================================================
    
private:

    const uint32_t k_safeStateMinDt = 500000;
    const uint32_t k_selfRightTriggerDt = 3000000;
    
    //  ====================================================================
    //
    //  Private members
    //
    //  ====================================================================
    
    TurretRotationController *m_pTurretRotationController;
    HammerController *m_pHammerController;
    FlameThrowerController *m_pFlameThrowerController;
    IMUController *m_pIMUController;
    AutoFire *m_pAutoFireController;

    controllerState m_state;
    uint32_t m_lastUpdateTime;
    uint32_t m_stateStartTime;

    Params m_params;

};

extern TurretController Turret;