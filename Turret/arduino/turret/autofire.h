#pragma once
#include <stdint.h>
#include "track.h"

enum AutoFireState {
    AF_NO_TARGET = 0,
    AF_OMEGAZ_LOCKOUT,
    AF_NO_HIT,
    AF_HIT
};

enum AutoFireState updateAutoFire(const Track &tracked_object,
                           int16_t depth, int16_t hammer_intensity);

bool omegaZLockout(int16_t *omegaZ);

void setAutoFireParams(int16_t p_xtol,
                       int16_t p_ytol,
                       int16_t p_max_omegaz,
                       uint32_t telemetry_interval);

void restoreAutoFireParameters(void);

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class AutoFire
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
        int32_t max_omegaZ;   // rad/s * 2048 = 50 deg/sec
        uint32_t autofire_telem_interval;
    } __attribute__((packed));

    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();

   void SetParams(int16_t p_xtol, int16_t p_ytol, int16_t p_max_omegaz, uint32_t p_telemetry_interval);
    void RestoreParams();

    void SendTelem();

private: 

    enum autoFireState 
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
    uint32_t m_stateStartTime;

    uint32_t m_lastAutoFireTelem = 0;

    Params m_params;
};