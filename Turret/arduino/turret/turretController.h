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
class SelfRightController;

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

    bool Nominal();
    Track* GetCurrentTarget();
    int32_t GetCurrentSpeed();
    int32_t GetDesiredManualTurretSpeed();

    void SetAutoAimParameters(int32_t p_proportionalConstant, int32_t p_derivativeConstant, int32_t p_steer_max, int32_t p_gyro_gain, uint32_t telemetry_interval);
    void SetAutoFireParameters(int16_t p_xtol, int16_t p_ytol, int16_t p_max_omegaz, uint32_t telemetry_interval);

    void SetParams(uint32_t p_watchDogTimerTriggerDt);
    void RestoreParams();

    void SendTelem();

private:

    enum controllerState 
    {
        EInit,
        ESafe,

        ENominal,

        EHammerTrigger,
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
    SelfRightController *m_pSelfRightController;

    controllerState m_state;
    uint32_t m_lastUpdateTime;
    uint32_t m_stateStartTime;

    Params m_params;

};

extern TurretController Turret;