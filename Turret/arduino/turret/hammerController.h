#pragma once

//  ====================================================================
//
//  Forward declerations
//
//  ====================================================================

class AutoFire;

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class HammerController
{

    //  ====================================================================
    //
    //  Internal structure decleration
    //
    //  ====================================================================

public:

    struct Params
    {
        int32_t tmp;
    };

    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();

    void Fire();
    void FireSelfRight();


    void SetAutoFireParameters(int16_t p_xtol, int16_t p_ytol, int16_t p_max_omegaz, uint32_t telemetry_interval);
    void SetParams(uint32_t p_manualControlOverideSpeed);
    void RestoreParams();

    void SendTelem();

private:

    enum controllerState 
    {
        EInit,
        ESafe,
        EDisabled,

        EReadyToFire,
        EFire,
        EFireSelfRight,

        ERetractStart,
        ERetracting,
        ERetractComplete,

        ESwingStart,
        ESwingMeasure,
        ESwingComplete,
        ESwingTooFar,

        EInvalid = -1
    };

    //  ====================================================================
    //
    //  Private methods
    //
    //  ====================================================================

    void updateSpeed();

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
    const uint32_t k_swingTimeMaxDt = 1000000;
    const uint32_t k_swingUpdateDt = 1000;

    //  ====================================================================
    //
    //  Private members
    //
    //  ====================================================================
    
    controllerState m_state;
    uint32_t m_stateStartTime;

    uint32_t m_swingStartTime;

    AutoFire* m_pAutoFire;

    Params m_params;
};