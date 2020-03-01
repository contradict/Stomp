#pragma once

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

    int32_t GetDesiredAutoAimSpeed();
    int32_t GetDesiredSBusSpeed();

    void SetParams(uint32_t p_watchDogTimerTriggerDt);
    void RestoreParams();

    void SendTelem();
    
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
    
    enum controllerState 
    {
        EInit,
        ESafe,
        EActive,

        EInvalid = -1
    };

    TurretRotationController *m_pTurretRotationController;

    controllerState m_state;
    uint32_t m_stateStartTime;

    Params m_params;

    //  ====================================================================
    //
    //  Private methods
    //
    //  ====================================================================

    void initAllControllers();
    
    void setState(controllerState p_state);

    void saveParams();
};

extern TurretController Turret;