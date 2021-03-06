#pragma once

#include "autoAimController.h"

//  ====================================================================
//
//  Forward declerations
//
//  ====================================================================

class TurretController;

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
    void Safe();
    
    int32_t GetMotorSpeed();
    int16_t GetTurretAngle();

    void SetAutoAimParameters(int32_t p_proportionalConstant, int32_t p_derivativeConstant, int32_t p_steer_max, int32_t p_gyro_gain);
    void SetParams(uint32_t p_manualControlOverideSpeed);
    void RestoreParams();

    void SendTelem();
    
private:

    enum controllerState 
    {
        EInit,
        ESafe,
        EDisabled,
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
    void updateAngle();

    void setState(controllerState p_state);
    void setSpeed(int32_t p_speed);

    void init();
    void initRoboTeq();

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
    uint32_t m_lastUpdateTime;
    uint32_t m_stateStartTime;

    int32_t m_motorSpeedCurrent;
    int16_t m_turretAngleCurrent;

    AutoAimController* m_pAutoAim;

    Params m_params;
};