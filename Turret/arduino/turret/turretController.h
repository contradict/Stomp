#pragma once

#include "track.h"

//  ====================================================================
//
//  Forward declerations
//
//  ====================================================================

class TurretRotationController;

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

    Track* GetCurrentTarget();
    int32_t GetDesiredManualTurretSpeed();

    void SetParams(uint32_t p_watchDogTimerTriggerDt);
    void RestoreParams();

    void SendTelem();

private:

    enum controllerState 
    {
        EInit,
        ESafe,
        EActive,

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

    //  ====================================================================
    //
    //  Private members
    //
    //  ====================================================================
    
    TurretRotationController *m_pTurretRotationController;

    controllerState m_state;
    uint32_t m_stateStartTime;

    Params m_params;

};

extern TurretController Turret;