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
    bool IsFlameOnEnabled();
    bool IsFlamePulseEnabled();

    bool IsHammerSwingRequested();
    bool IsHammerRetractRequested();

    int32_t GetDesiredManualTurretSpeed();

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