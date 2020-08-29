#pragma once

//  ====================================================================
//
//  Forward declerations
//
//  ====================================================================

class AutoFireController;

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
        int16_t swingTelemetryFrequency;
    } __attribute__((packed));

    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();
    void Safe();
    
    bool ReadyToSwing();

    void TriggerSwing();
    void TriggerSelfRightSwing();
    void TriggerRetract();

    int32_t GetHammerSpeed();
    int16_t GetHammerAngle();

    void SetParams(uint32_t p_selfRightIntensity, uint32_t p_swingTelemetryFrequency);
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

    void updateSpeed();
    void updateAngle();
    void updatePressure();
    
    void setState(controllerState p_state);
    void init();

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

    uint8_t m_throwPressureAngle;
    
    Params m_params;
};