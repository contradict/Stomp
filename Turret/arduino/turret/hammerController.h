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
        int16_t selfRightIntensity;
        int16_t telemetryFrequency;
    };

    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();

    bool ReadyToFire();

    void Fire();
    void FireSelfRight();
    void Retract();

    void SetAutoFireParameters(int16_t p_xtol, int16_t p_ytol, int16_t p_max_omegaz, uint32_t telemetry_interval);
    void SetParams(uint32_t p_selfRightIntensity, uint32_t p_telemetryFrequency);
    void RestoreParams();

    void SendTelem();

private:

    enum controllerState 
    {
        EInit,
        ESafe,
        EDisabled,
        EReady,

        EThrow,
        EThrowSelfRight,
        ERetract,

        EFullCycleInterruptMode,
        ERetractOnlyInterruptMode,

        ESwingComplete,
        
        EInvalid = -1
    };

    //  ====================================================================
    //
    //  Private methods
    //
    //  ====================================================================

    void setState(controllerState p_state);
    void initAllControllers();

    void resetTelem();
    void saveParams();

    //  ====================================================================
    //
    //  Private constants
    //
    //  ====================================================================
    
private:

    const uint32_t k_throwPressureAngleSelfRight = 30;
    
    const uint32_t k_safeStateMinDt = 500000;
    const uint32_t k_swingTimeMaxDt = 1000000;
    const uint32_t k_swingUpdateDt = 1000;

    const uint8_t k_throwIntensityToAngle[9] = { 3, 5, 10, 15, 20, 30, 40, 50, 65 };

    //  ====================================================================
    //
    //  Private members
    //
    //  ====================================================================
    
    controllerState m_state;
    uint32_t m_lastUpdateTime;
    uint32_t m_stateStartTime;

    bool m_retractOnlyPhase;

    uint32_t m_throwStartTime;
    uint8_t m_throwPressureAngle;

    AutoFire* m_pAutoFire;

    Params m_params;
};