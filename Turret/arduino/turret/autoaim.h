#pragma once

#include "track.h"

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class AutoAim
{

    //  ====================================================================
    //
    //  Internal structure decleration
    //
    //  ====================================================================

public:

    struct Params 
    {
        int32_t proportionalConstant;
        int32_t derivativeConstant;
        int32_t speedMax;
        int32_t gyro_gain;
        uint32_t autoaimTelemInterval;
    } __attribute__((packed));

    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();

    int32_t GetDesiredTurretSpeed();

    void SetParams(int32_t p_proportionalConstant, int32_t p_derivativeConstant, int32_t p_steer_max, int32_t p_gyro_gain, uint32_t p_telemetry_interval);
    void RestoreParams();
    
    void SendTelem();

private: 

    enum autoAimState 
    {
        EInit = 0,
        ESafe,
        EDisabled,
        ENoTarget,
        ETrackingTarget,

        EInvalid = -1
    };

    //  ====================================================================
    //
    //  Private methods
    //
    //  ====================================================================

    void updateDesiredTurretSpeed();

    void setState(autoAimState p_state);

    void saveParams();

    //  ====================================================================
    //
    //  Private constants
    //
    //  ====================================================================
    
    const uint32_t k_safeStateMinDt = 500000;

private:

    //  ====================================================================
    //
    //  Private members
    //
    //  ====================================================================

    autoAimState m_state;
    uint32_t m_stateStartTime;

    Track *m_pTarget;

    int32_t m_desiredTurretSpeed = 0;
    uint32_t m_lastAutoaimTelem = 0;

    Params m_params;
};