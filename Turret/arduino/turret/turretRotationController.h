#pragma once

#include "autoaim.h"

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class TurretRotationController
{

    //  ====================================================================
    //
    //  Internal structure decleration
    //
    //  ====================================================================

public:

    struct Params
    {
        int32_t ManualControlOverideSpeed;
    };

    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();

    int32_t GetCurrentSpeed();

    void SetParams(uint32_t p_manualControlOverideSpeed);
    void RestoreParams();

    void SendTelem();
    
private:

    enum controllerState 
    {
        EInit,
        ESafe,
        EDisabled,
        EIdle,
        EManualControl,
        EAutoAim,
        EAutoAimWithManualAssist,

        EInvalid = -1
    };

    //  ====================================================================
    //
    //  Private methods
    //
    //  ====================================================================

    void updateSpeed();

    void setState(controllerState p_state);
    void setSpeed(int32_t p_speed);

    void initAllControllers();
    void initMotorController();

    void saveParams();

    //  ====================================================================
    //
    //  Private constants
    //
    //  ====================================================================
    
private:

    const int32_t k_maxSpeed = 1000;
    const int32_t k_minSpeed = -1000;

    const uint32_t k_safeStateMinDt = 500000;

    //  ====================================================================
    //
    //  Private members
    //
    //  ====================================================================
    
    controllerState m_state;
    uint32_t m_stateStartTime;

    int32_t m_currentSpeed;

    AutoAim *m_pAutoAim;

    Params m_params;
};