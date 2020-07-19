#pragma once

#include <stdint.h>

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

    int16_t GetSize() const;
    int16_t GetAngle() const;
    int16_t GetXCoord() const;
    int16_t GetYCoord() const;
    int16_t GetRadius() const;
    
    inline int32_t DistanceSq(const Target &other) const;

    //  ====================================================================
    //
    //  Public Members
    //
    //  ====================================================================

    uint16_t SumDistance;
    int32_t SumIntensity;
    int32_t SumAngleIntensity;
    int8_t LeftEdge, RightEdge;
    uint32_t Time;

};

