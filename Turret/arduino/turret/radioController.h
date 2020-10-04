#pragma once

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class RadioController
{

    //  ====================================================================
    //
    //  Internal structure decleration
    //
    //  ====================================================================

public:

    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();

    bool IsNominal();

    bool IsWeaponEnabled();
    bool IsManualTurretEnabled();
    bool IsAutoAimEnabled();
    bool IsAutoFireEnabled();
    bool IsSelfRightEnabled();
    
    bool IsFlameRightOnEnabled();
    bool IsFlameRightPulseEnabled();

    bool IsHammerSwingRequested();
    bool IsHammerRetractRequested();

    int32_t GetDesiredManualTurretSpeed();

    int32_t GetHammerIntensity();
    int32_t GetHammerIntensityAngle();
    int32_t GetHammerStrikeDistance();

    void SendTelem();

private:

    enum controllerState 
    {
        EInit,
        ESafe,
        EDisabled,

        EInvalid = -1
    };

    //  ====================================================================
    //
    //  Private methods
    //
    //  ====================================================================

    void setState(controllerState p_state);

    void init();

    void sendTelem();

    //  ====================================================================
    //
    //  Private constants
    //
    //  ====================================================================
    
private:

    const int8_t k_hammerIntensitiesAngle[9] = { 3, 5, 10, 15, 20, 30, 40, 50, 65 };

    //  ====================================================================
    //
    //  Private members
    //
    //  ====================================================================
    
    controllerState m_state;
    uint32_t m_lastUpdateTime;
    uint32_t m_stateStartTime;
};

extern RadioController Radio;