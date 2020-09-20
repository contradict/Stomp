#pragma once

#include <stdint.h>

#include "leddar_io.h"

//  ====================================================================
//
//  Class decleration
//
//  ====================================================================

class Target
{
    //  ====================================================================
    //
    //  Public API
    //
    //  ====================================================================
 
public:

    Target() : SumDistance(0), LeftEdge(0), RightEdge(0), Time(0) { }

    void Reset();
    void StartSegment(int32_t p_startSegment, Detection* p_detection);
    void AddSegment(int32_t p_startSegment, Detection* p_detection);
    void EndSegment(int32_t p_endSegment, Detection* p_detection, uint32_t p_time);

    int16_t GetSize() const;
    int16_t GetAngle() const;
    int16_t GetXCoord() const;
    int16_t GetYCoord() const;
    int16_t GetDistance() const;
    
    inline int32_t DistanceSq(Target &other);

    //  ====================================================================
    //
    //  Public Members
    //
    //  ====================================================================

    enum EdgeFOVType
    {
        EInvalid,
        EBeingConstructed,
        ELeftAndRgihtEdgeOutOfFOV,
        ELeftEdgeOutOfFOV,
        ERightEdgeOutOfFOV,
        EInFOV
    };

    uint32_t SumDistance;
    int32_t SumIntensity;
    int32_t SumAngleIntensity;
    int8_t LeftEdge, RightEdge;
    uint32_t Time;
    EdgeFOVType Type;
};

