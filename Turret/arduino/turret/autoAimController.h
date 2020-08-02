#pragma once

#include "fixedpoint.h"

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class AutoAimController
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
        int32_t integralConstant;
        int32_t derivativeConstant;
        int32_t speedMax;
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

    void SetParams(int32_t p_proportionalConstant, int32_t p_integralConstant, int32_t p_derivativeConstant, int32_t p_speedMax);
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

    void init();
    
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
    uint32_t m_lastUpdateTime;
    uint32_t m_latestDt;

    uint32_t m_stateStartTime;

    FP_32x20 m_error;
    FP_32x20 m_errorIntegral;
    FP_32x20 m_errorDerivative;


    int32_t m_desiredTurretSpeed = 0;
    uint32_t m_lastAutoaimTelem = 0;

    Params m_params;
};