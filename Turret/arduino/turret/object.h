#pragma once
#include <stdint.h>
struct Object
{
    uint16_t SumDistance;
    int32_t SumIntensity;
    int32_t SumAngleIntensity;
    int8_t LeftEdge, RightEdge;
    uint32_t Time;

    // Default constructor
    Object() : SumDistance(0),
               LeftEdge(0), RightEdge(0),
               Time(0) { }
    int16_t size(void) const;
    int16_t angle(void) const;
    int16_t xcoord(void) const;
    int16_t ycoord(void) const;
    int16_t radius(void) const;
    inline int32_t distanceSq(const Object &other) const;
};

