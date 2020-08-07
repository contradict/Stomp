#pragma once

#include <stdint.h>

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class AutoFireController
{

    //  ====================================================================
    //
    //  Internal structure decleration
    //
    //  ====================================================================

public:

    struct Params 
    {
        int32_t xtol, ytol;
        int32_t maxOmegaZ;   // rad/s * 2048 = 50 deg/sec
    } __attribute__((packed));

    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();

    bool ShouldSwingAtTarget();
    
    void SetParams(int16_t p_xtol, int16_t p_ytol, int16_t p_maxOmegaZ);
    void RestoreParams();

    void SendTelem();

private: 

    enum autoFireState 
    {
        EInit = 0,
        ESafe,
        EDisabled,
        ERotationLockout,
        ENoTarget,
        ETrackingTarget,
        ESwingAtTarget,

        EInvalid = -1
    };

    //  ====================================================================
    //
    //  Private methods
    //
    //  ====================================================================

    bool willHitTarget();

    void setState(autoFireState p_state);

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

    autoFireState m_state;
    uint32_t m_lastUpdateTime;
    uint32_t m_stateStartTime;

    uint32_t m_lastAutoFireTelem = 0;

    Params m_params;
};
