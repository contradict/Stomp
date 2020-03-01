#pragma once

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

    void SetParams(uint32_t p_manualControlOverideSpeed);
    void RestoreParams();

    void SendTelem();
    
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
    
    enum controllerState 
    {
        EInit,
        ESafe,
        EIdle,
        EManualControl,
        EAutoAim,
        EAutoAimWithManualAssist,

        EInvalid = -1
    };

    controllerState m_state;
    uint32_t m_stateStartTime;

    int32_t m_currentSpeed;

    Params m_params;

    //  ====================================================================
    //
    //  Private methods
    //
    //  ====================================================================

    void initMotorController();

    void updateSpeed();

    void setState(controllerState p_state);
    void setSpeed(int32_t p_speed);

    void saveParams();
};