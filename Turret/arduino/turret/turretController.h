#pragma once

//  ====================================================================
//
//  Forward declerations
//
//  ====================================================================

class TurretRotationController;
class TargetTrackingController;
class HammerController;
class FlameThrowerController;
class IMUController;
class AutoFireController;

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class TurretController
{

    //  ====================================================================
    //
    //  Internal structure decleration
    //
    //  ====================================================================

public:

    struct Params
    {
        int32_t WatchDogTimerTriggerDt;
    };

    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();

    bool IsNominal();

    int16_t GetTurretRotationSpeed();
    int16_t GetTurretAngle();
    
    int32_t GetHammerSpeed();
    int16_t GetHammerAngle();

    int32_t GetEstimatedSwingDuration();

    void FlamePulseStart();
    void FlamePulseStop();

    //  Turret methods that pass along parameter setting to the controllers that the turret owns

    void SetAutoAimParameters(int32_t p_proportionalConstant, int32_t p_derivativeConstant, int32_t p_steer_max, int32_t p_gyro_gain);
    void SetAutoFireParameters(int16_t p_xtol, int16_t p_ytol, int16_t p_max_omegaz);
    void SetHammerParameters(uint32_t p_selfRightIntensity, uint32_t p_swingTelemetryFrequency);
    void SetTurretRotationParameters(uint32_t p_manualControlOverideSpeed);
    void SetIMUParameters(int8_t p_dlpf, int32_t p_imuPeriod, int32_t p_stationaryThreshold,
        int16_t p_uprightCross, int16_t p_minValidCross, int16_t p_maxValidCross,
        int16_t p_maxTotalNorm, int16_t p_xThreshold, int16_t p_zThreshold);

    void SetParams(uint32_t p_watchDogTimerTriggerDt);
    void RestoreParams();

    void SendTelem();

private:

    enum controllerState 
    {
        EInit,
        ESafe,

        ENominal,

        EHammerTriggerThrowRetract,
        EHammerTriggerRetract,
        EHammerActive,

        ENeedsSelfRight,
        ESelfRightTrigger,

        EUnknown,
        EDegraded,

        EInvalid = -1
    };

    //  ====================================================================
    //
    //  Private methods
    //
    //  ====================================================================

    void setState(controllerState p_state);

    void init();
    void initAllControllers();

    void saveParams();

    //  ====================================================================
    //
    //  Private constants
    //
    //  ====================================================================
    
private:

    const uint32_t k_safeStateMinDt = 500000;
    const uint32_t k_selfRightTriggerDt = 3000000;
    
    //  ====================================================================
    //
    //  Private members
    //
    //  ====================================================================
    

    controllerState m_state;
    uint32_t m_lastUpdateTime;
    uint32_t m_stateStartTime;

    //  All of the controllers owned by the Turret

    TurretRotationController *m_pTurretRotationController;
    HammerController *m_pHammerController;
    FlameThrowerController *m_pFlameThrowerController;
    IMUController *m_pIMUController;
    AutoFireController *m_pAutoFireController;

    Params m_params;

};

extern TurretController Turret;