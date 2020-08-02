#pragma once

#include "target.h"

//  ====================================================================
//
//  Forward declerations
//
//  ====================================================================

class TargetAcquisitionController;

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class TargetTrackingController
{

    //  ====================================================================
    //
    //  Internal structure decleration
    //
    //  ====================================================================

public:

    struct Params
    {
        int16_t alpha;
        int16_t beta;                       // position, velocity filter
        uint32_t trackLostDt;               // timeout for no observations
        uint32_t minNumUpdates;             // minimum number before trusted
        int32_t maxOffTrackDistanceSq;      // squared distance in mm
        int32_t maxStartDistanceSq;         // squared distance in mm
    };

    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();
    
    bool IsTrackingValidTarget();
    bool WillHitTrackedTarget();

    int32_t GetTargetErrorAngle();
    int32_t GetDistanceSqToTarget(const Target &p_target);

    void SetObjectSegmentationParams(int32_t p_objectSizeMin, 
        int32_t p_objectSizeMax, 
        int32_t p_edgeCallThreshold, 
        bool p_closestOnly);

    void SetParams(int16_t p_alpha, 
        int16_t p_beta,
        int8_t p_minNumUpdates,
        uint32_t p_trackLostDt,
        int16_t p_maxOffTrackDistance,
        int16_t p_maxStartDistance);
                             
    void RestoreParams();

    void SendTelem();
    void SendLeddarTelem();
    
private:

    enum controllerState 
    {
        EInit,
        ENoTarget,
        ETargetAcquired,
        ETargetTracked,
  
        EInvalid = -1
    };

    //  ====================================================================
    //
    //  Private methods
    //
    //  ====================================================================

    void predictedTrackedTargetLocation();
    void updateTracking();

    void project(int32_t dt, int32_t dtheta, int32_t *p_px, int32_t *p_py);

    int32_t getAngle();
    int32_t getVTheta(); 

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

    //  ====================================================================
    //
    //  Private members
    //
    //  ====================================================================
    
    controllerState m_state;
    uint32_t m_lastUpdateTime;
    uint32_t m_stateStartTime;
    uint32_t m_latestDt;

    TargetAcquisitionController *m_pTargetAcquisitionController;
    Target *m_pTrackedTarget;

    int32_t m_x;
    int32_t m_vx;
    int32_t m_y;
    int32_t m_vy;
    int32_t m_residualX;
    int32_t m_residualY;
    uint32_t m_numUpdates;
    int32_t m_lastOmgaZ;

    Params m_params;
};

extern TargetTrackingController TargetTracking;