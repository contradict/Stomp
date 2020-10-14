#pragma once

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class FlameThrowerController
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

    void Safe();
    void Enable();

    void FlamePulseStart();
    void FlamePulseStop();

private:

    enum controllerState 
    {
        EInit,
        ESafe,
        
        EDisabled,
        EReadyToFire,

        EPulseFlameOn,
        EManualFlameOn,
 
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
    void saveParams();

    //  ====================================================================
    //
    //  Private constants
    //
    //  ====================================================================
    
private:

    const uint32_t k_safeStateMinDt = 500000;

    //  ====================================================================
    //
    //  Private members
    //
    //  ====================================================================
    
    controllerState m_state;
    uint32_t m_lastUpdateTime;
    uint32_t m_stateStartTime;
};