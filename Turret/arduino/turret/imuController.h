#pragma once

//  ====================================================================
//
//  Forward declerations
//
//  ====================================================================

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class IMUController
{

    //  ====================================================================
    //
    //  Internal structure decleration
    //
    //  ====================================================================

public:

    struct Params
    {
        int8_t dlpfMode;
        uint32_t imuPeriod;
        int32_t stationaryThreshold;
        int16_t uprightCross;
        int16_t minValidCross;
        int16_t maxValidCross;
        int16_t maxTotalNorm;
        int16_t xThreshold;
        int16_t zThreshold;
    } __attribute__((packed));

    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();

    int16_t GetOmegaZ();
    bool IsUpright();
    bool IsUpsideDown();

    void SetParams(int8_t p_dlpf, int32_t p_imuPeriod, int32_t p_stationaryThreshold,
        int16_t p_uprightCross, int16_t p_minValidCross, int16_t p_maxValidCross,
        int16_t p_maxTotalNorm, int16_t p_xThreshold, int16_t p_zThreshold);

    void RestoreParams();

    void SendTelem();
    
private:

    enum controllerState 
    {
        EInit,
        ESafe,

        EUnknown,
        EUpright,
        EUpsideDown,

        EInvalid = -1
    };

    //  ====================================================================
    //
    //  Private methods
    //
    //  ====================================================================

    void queryIMU();

    bool isPossiblyStationary();
    bool doesMathSayUpright();
    bool doesMathSayNotUpright();

    void setState(controllerState p_state);

    void init();

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
    uint32_t m_lastIMUQueryTime;

    bool m_queryIMUFailed;

    int16_t m_acceleration[3];
    int16_t m_angularRate[3];
    int16_t m_temperature;
    int32_t m_sumAngularRate;
    int16_t m_crossNorm;
    int16_t m_totalNorm;

    Params m_params;
};