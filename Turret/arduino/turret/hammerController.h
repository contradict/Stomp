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
        int16_t maxThrowAngle;
        int16_t minRetractAngle;
        uint32_t maxThrowUnderPressureDt;
        uint32_t maxThrowExpandDt;
        uint32_t maxRetractUnderPressureDt;
        uint32_t maxRetractExpandDt;
        uint32_t maxRetractBreakDt;
        int32_t velocityFilterCoefficient;
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

    void SetParams(uint32_t p_selfRightIntensity, 
        uint32_t p_swingTelemetryFrequency,
        uint16_t p_maxThrowAngle,
        uint16_t p_minRetractAngle,
        uint32_t p_maxThrowUnderPressureDt,
        uint32_t p_maxThrowExpandDt,
        uint32_t p_maxRetractUnderPressureDt,
        uint32_t p_maxRetractExpandDt,
        uint32_t p_maxRetractBreakDt);

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

    const int16_t k_throwIntensityToAngle[9] = { 52, 87, 174, 261, 349, 30, 523, 872, 1134 };

    //  ====================================================================
    //
    //  Private members
    //
    //  ====================================================================
    
    controllerState m_state;
    uint32_t m_lastUpdateTime;
    uint32_t m_stateStartTime;

    int16_t m_throwPressureAngle;
    
    Params m_params;
};
