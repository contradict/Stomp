#pragma once

#include "target.h"
#include "leddar_io.h"

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

class TargetAcquisitionController
{

    //  ====================================================================
    //
    //  Internal structure decleration
    //
    //  ====================================================================

public:

    struct Params
    {
        int32_t objectSizeMin;      // object sizes are in mm
        int32_t objectSizeMax;      // mm of circumferential size
        int32_t edgeCallThreshold;  // cm for edge in leddar returns
        bool closestOnly;           // Only inspect closest object
    };

    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    void Init();
    void Update();
    
    Target* GetBestTarget();

    void SetParams(int32_t p_objectSizeMin, int32_t p_objectSizeMax, int32_t p_edgeCallThreshold, bool closestOnly);
    void RestoreParams();

    void SendTelem();
    void SendLeddarTelem();
    
private:

    enum controllerState 
    {
        EInit,
        ENoTargets,
        ETargetAcquired,
  
        EInvalid = -1
    };

    //  ====================================================================
    //
    //  Private methods
    //
    //  ====================================================================

    void updateBestTarget();

    void segmentTargets();
    void selectTarget();
    void selectClosestTarget();

    void setState(controllerState p_state);

    void init();

    void saveParams();

    //  ====================================================================
    //
    //  Private constants
    //
    //  ====================================================================
    
    static const int32_t k_leddarRequestMaxDt = 100000L;

private:

    //  ====================================================================
    //
    //  Private members
    //
    //  ====================================================================
    
    controllerState m_state;
    uint32_t m_lastUpdateTime;
    uint32_t m_stateStartTime;
    uint32_t m_lastRequestDetectionsTime;

    Target* m_pBestTarget;

    uint32_t m_rawDetectionCount;
    Detection (*m_minDetections)[LEDDAR_SEGMENTS];

    Target m_possibleTargets[8];
    uint32_t m_possibleTargetsCount;

    // LeddarController* m_pLeddarController;
    Params m_params;
};